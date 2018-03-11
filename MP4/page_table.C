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
    // allocate pte
    page_table = (unsigned long *)(kernel_mem_pool->get_frames(1) * PAGE_SIZE);
    page_directory[0] = (unsigned long)page_table + 0x03; // kernel mode, R/W, Present
    for(int i=1;i<ENTRIES_PER_PAGE;i++)
    {
      page_directory[i] = 0; // kernel mode, R/W, not present
    }

    // set up pte
    for(int i=0;i<ENTRIES_PER_PAGE;i++)
    {
      page_table[i] = (i<<12) + 0x03; // kernel mode, R/W, Present
    }
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
    write_cr3((unsigned long)page_directory);
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
      unsigned long *page_table = (unsigned long *)(current_page_table->page_directory[ptd_offset]);
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
      unsigned long new_frame;
      // check ptd entry
      // ptd entry not present
      if((current_page_table->page_directory[ptd_offset] & 0x1) == 0)
      {
          // pte must not exist, first allocate page table
          if((new_frame = kernel_mem_pool->get_frames(1)) == 0)
          {
              Console::puts("no frame in kernel pool available for page table\n");
              assert(false);
          }
          unsigned long *page_table = (unsigned long *)(new_frame * PAGE_SIZE);
          current_page_table->page_directory[ptd_offset] = (unsigned long)page_table + 0x3;
          
          // set up pte
          for(int i=0;i<ENTRIES_PER_PAGE;i++)
          {
              page_table[i] = 0; // not present
          }
          if((new_frame = process_mem_pool->get_frames(1)) == 0)
          {
              Console::puts("no frame in process pool available for faulted page\n");
              assert(false);
          }
          page_table[pte_offset] = new_frame * PAGE_SIZE + 0x1;
      }
      // pte not preset
      else 
      {
          unsigned long *page_table = (unsigned long *)((current_page_table->page_directory[ptd_offset]) & 0xfffff000);
          if((page_table[pte_offset] & 0x1) == 0)
          {
              if((new_frame = process_mem_pool->get_frames(1)) == 0)
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
    Console::puts("handled page fault\n");
}

void PageTable::register_pool(VMPool * _vm_pool)
{
    assert(false);
    Console::puts("registered VM pool\n");
}

void PageTable::free_page(unsigned long _page_no) {
    assert(false);
    Console::puts("freed page\n");
}
