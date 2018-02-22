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
   process_mem_pool = _process_mem_pooll
   shared_size = _shared_size;
   Console::puts("Initialized Paging System\n");
}

PageTable::PageTable()
{
   unsigned long page_table;
   unsigned long prot_mask = 0x03; // kernel mode, R/W, Present
   current_page_table = this;
   // allocate and setup ptd
   page_directory = kernel_mem_pool->get_frames(1);
   // allocate pte
   page_table = kernel_mem_pool->get_freams(1);
   *page_directory = page_table<<12 + prot_mask;

   // set up pte
   for(int i=0;i<PT_ENTRIES_PER_PAGE;i++)
   {
      *(page_table+i) = i<<12 + prot_mask;
   }
   Console::puts("Constructed Page Table object\n");
}


void PageTable::load()
{
   assert(false);
   Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
   assert(false);
   Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{
  assert(false);
  Console::puts("handled page fault\n");
}

