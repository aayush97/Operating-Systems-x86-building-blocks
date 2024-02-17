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


// void i_to_a(unsigned long i, char * a)
// {
//    int j = 0;
//    while (i > 0)
//    {
//       a[j] = (i % 10) + '0';
//       i = i / 10;
//       j++;
//    }
//    a[j] = 0;
//    int k = 0;
//    j--;
//    while (k < j)
//    {
//       char c = a[k];
//       a[k] = a[j];
//       a[j] = c;
//       k++;
//       j--;
//    }
// }

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
   char a[20];
   // i_to_a((unsigned long) current_page_table->page_directory, a);
   // Console::puts("Page Directory at ");
   // Console::puts(a);
   // Console::puts("\n");
   // Console::puts("Page Table at ");
   // i_to_a((unsigned long)page_table, a);
   // Console::puts(a);
   // Console::puts("\n");
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
   Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
   char a[20];
   // i_to_a((unsigned long) current_page_table->page_directory, a);
   // Console::puts("Enabling paging with page directory at ");
   // Console::puts(a);
   // Console::puts("\n");
   write_cr3((unsigned long) current_page_table->page_directory);
   write_cr0(read_cr0() | 0x80000000);
   paging_enabled = 1;
   Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{
  assert(false);
  Console::puts("handled page fault\n");
}

