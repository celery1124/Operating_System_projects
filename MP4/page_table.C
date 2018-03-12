#include "assert.H"
#include "exceptions.H"
#include "console.H"
#include "paging_low.H"
#include "page_table.H"

PageTable * PageTable::current_page_table = NULL;
unsigned int PageTable::paging_enabled = 0;
ContFramePool * PageTable::kernel_mem_pool = NULL;
ContFramePool * PageTable::process_mem_pool = NULL;
unsigned long PageTable::shared_size = 0;


void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                            ContFramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{
    kernel_mem_pool = _kernel_mem_pool;
    process_mem_pool = _process_mem_pool;
    shared_size = _shared_size;
    Console::puts("Initialized Paging System\n");
}

PageTable::PageTable()
{
    unsigned long *page_table;
    // allocate and setup ptd
    page_directory = (unsigned long *)(kernel_mem_pool->get_frames(1) * PAGE_SIZE);
    // allocate page table pages from process pool
    page_table = (unsigned long *)(process_mem_pool->get_frames(1) * PAGE_SIZE);
    page_directory[0] = (unsigned long)page_table + 0x03; // kernel mode, R/W, Present
    for(int i=1;i<ENTRIES_PER_PAGE - 1;i++)
    {
      page_directory[i] = 0; // kernel mode, R/W, not present
    }
    // for reverse look up
    page_directory[ENTRIES_PER_PAGE-1] = (unsigned long)page_directory + 0x03; //kernel mode, R/W Present

    // page table page in the process pool need reverse lookup
    page_table = (unsigned long *)(1023<<22); // first entry in page directory
    for(int i=0;i<ENTRIES_PER_PAGE;i++)
    {
      page_table[i] = (i<<12) + 0x03; // kernel mode, R/W, Present
    }

    // init VMPool list
    head = NULL;
    Console::puts("Constructed Page Table object\n");
}


void PageTable::load()
{
    current_page_table = this;
    write_cr3((unsigned long)page_directory);
    Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
    write_cr0((unsigned long)(read_cr0() | 0x80000000));
    paging_enabled = 1;
    Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{
    unsigned long fault_addr = read_cr2();
    int ptd_offset = fault_addr>>22;
    int pte_offset = (fault_addr<<10)>>22;
    // handle protection fault
    if((_r->err_code & 0x1) == 1)
    {
        // reverse look up for page table pages
        unsigned long *page_table = (unsigned long *)((1023<<22) | (ptd_offset << 12));
        int pte_flag = page_table[pte_offset] & 0xC;
        if(_r->err_code & 0x4 == 0) // we are in kernel reference mode
        {
          if((_r->err_code & 0x2) > (pte_flag & 0x2))
          {
              Console::puts("kernel touch Read only page!\n");
              assert(false);
          }
        }
        else // we are in user reference mode
        {
          if((_r->err_code & 0x4) > (pte_flag & 0x2))
          {
              Console::puts("user touch kernel page!\n");
              assert(false);
          }
          else if ((_r->err_code & 0x2) > (pte_flag & 0x2))
          {
              Console::puts("user touch Read only page!\n");
              assert(false);
          }
        }
    }
    // handle not present fault
    else
    {
      ContFramePool *frame_pool = NULL;
      // check fault address legitimate
      VMPool *p = current_page_table->head;
      while(p != NULL)
      {
          if(p->is_legitimate(fault_addr))
          {
              frame_pool = p->frame_pool;
              break;
          }
          p = p->next;
      }
      if(frame_pool == NULL)
      {
          Console::puts("Segmentation fault\n");
          assert(false);
      }

      unsigned long new_frame;
      // check ptd entry
      // ptd entry not present
      if((current_page_table->page_directory[ptd_offset] & 0x1) == 0)
      {
          // pte must not exist, first allocate page table
          if((new_frame = frame_pool->get_frames(1)) == 0)
          {
              Console::puts("no frame in kernel pool available for page table\n");
              assert(false);
          }
          unsigned long *page_table = (unsigned long *)(new_frame * PAGE_SIZE);
          current_page_table->page_directory[ptd_offset] = (unsigned long)page_table + 0x3;
          
          // nned reverse lookup
          page_table = (unsigned long *)((1023<<22) | (ptd_offset << 12));
          // set up pte
          for(int i=0;i<ENTRIES_PER_PAGE;i++)
          {
              page_table[i] = 0; // not present
          }
          if((new_frame = frame_pool->get_frames(1)) == 0)
          {
              Console::puts("no frame in process pool available for faulted page\n");
              assert(false);
          }
          page_table[pte_offset] = new_frame * PAGE_SIZE + 0x1;
      }
      // pte not preset
      else 
      {
          unsigned long *page_table = (unsigned long *)((1023<<22) | (ptd_offset << 12));
          if((page_table[pte_offset] & 0x1) == 0)
          {
              if((new_frame = frame_pool->get_frames(1)) == 0)
              {
                  Console::puts("no frame in process pool available for faulted page\n");
                  assert(false);
              }
              page_table[pte_offset] = new_frame * PAGE_SIZE + 0x1; // kernel R/W
          }
          else
          {
              Console::puts("something wrong in page fault handler\n");
              assert(false);
          }
      }
    }
    Console::puts("handled page fault\n");
}

void PageTable::register_pool(VMPool * _vm_pool)
{
    if(head == NULL)
        head = _vm_pool;
    else
    {
        VMPool *p = head;
        while (p->next != NULL) p = p->next;
        p->next = _vm_pool;
    }
    Console::puts("registered VM pool\n");
}

void PageTable::free_page(unsigned long _page_no) {
    int ptd_offset = _page_no>>10;
    int pte_offset = _page_no & 0x3FF;
    // reverse look up for page table pages
    unsigned long *page_table = (unsigned long *)((1023<<22) | (ptd_offset << 12));
    unsigned long frame_no;
    // check pte is valid or not
    if(page_table[pte_offset] & 0x1 == 1)
    {
        frame_no = page_table[pte_offset] >> 12;
        // release physical frame
        ContFramePool::release_frames(frame_no);
        Console::puts("freed page\n");
        // clear pte
        page_table[pte_offset] = 0;
        // flush TLB
        write_cr3((unsigned long)page_directory);
        Console::puts("Flush TLB entries\n");
    }
    // pte not valid
    // do nothing
}
