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
   // Allocate a page directory
   page_directory = (unsigned long *)(kernel_mem_pool->get_frames(1) * Machine::PAGE_SIZE);

   // Map the first 4MB of memory to the same physical addresses
   unsigned long *page_table = (unsigned long *)(kernel_mem_pool->get_frames(1) * Machine::PAGE_SIZE);

   page_directory[0] = ((unsigned long)page_table) | 3;
   for (unsigned int i = 0; i < Machine::PT_ENTRIES_PER_PAGE; i++)
   {
      page_table[i] = (i << 12) | 3;
   }
   for (unsigned int i = 1; i < Machine::PT_ENTRIES_PER_PAGE; i++)
   {
      page_directory[i] = 0 | 2;
   }
   Console::puts("Constructed Page Table object\n");
}


void PageTable::load()
{
   current_page_table = this;
   // load the page directory into the cr3 register
   write_cr3((unsigned long)page_directory);
   Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
   write_cr0(read_cr0() | 0x80000000);
   paging_enabled = 1;
   Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{
   // read the offending address
   unsigned long offending_addr = read_cr2();

   unsigned int pde_index = (offending_addr >> 22);
   unsigned int pte_index = (offending_addr >> 12) & 0x3FF;
   unsigned long * page_directory = current_page_table->page_directory;
   unsigned long * page_table;

   // check if the pde is valid and writable
   if ((page_directory[pde_index] & 1) == 0)
   {
      // generate a valid page table for this pde
      page_table = (unsigned long *)(kernel_mem_pool->get_frames(1) * Machine::PAGE_SIZE);
      page_directory[pde_index] = ((unsigned long)page_table) | 3;
      for (unsigned int i = 0; i < Machine::PT_ENTRIES_PER_PAGE; i++)
      {
         page_table[i] = 0 | 2;
      }
   }else if((page_directory[pde_index] & 2) == 0)
   {
      Console::puts("Page Directory Entry not writable\n");
      assert(false);
   }else{
      page_table = (unsigned long *) (page_directory[pde_index] & 0xFFFFF000);
   }

   if ((page_table[pte_index] & 1) == 0)
   {
      unsigned long frame =  process_mem_pool->get_frames(1) * Machine::PAGE_SIZE;
      page_table[pte_index] = frame | 3;
   }else if ((page_table[pte_index] & 2) == 0)
   {
      Console::puts("Page Table Entry not writable\n");
      assert(false);
   }

   Console::puts("Page Fault Handled\n");

}

