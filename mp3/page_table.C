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
   // Console::puts("Page Fault\n");
   // read the offending address
   unsigned long offending_addr = read_cr2();
   char a[20];
   // i_to_a(offending_addr, a);
   // Console::puts("Page Fault at ");
   // Console::puts(a);
   // Console::puts("\n");
   // assert(false);
   // find the page table entry
   unsigned int pde_index = (offending_addr >> 22);
   unsigned int pte_index = (offending_addr >> 12) & 0x3FF;
   unsigned long * page_directory = current_page_table->page_directory;
   unsigned long * page_table;

   // check if the pde is valid and writable
   if ((page_directory[pde_index] & 1) == 0)
   {
      // Console::puts("Page Directory Entry not valid\n");
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
   // i_to_a(pde_index, a);
   // Console::puts("Page Directory Entry at ");
   // Console::puts(a);
   // Console::puts("\n");

   // i_to_a((unsigned long) page_directory, a);
   // Console::puts("Page Directory at ");
   // Console::puts(a);
   // Console::puts("\n");

   // i_to_a(pte_index, a);
   // Console::puts("Page Table Entry at ");
   // Console::puts(a);
   // Console::puts("\n");

   // i_to_a((unsigned long) page_table, a);
   // Console::puts("Page Table at ");
   // Console::puts(a);
   // Console::puts("\n");

   // i_to_a((unsigned long) page_table[pte_index], a);
   // Console::puts("Page Table Entry Value ");
   // Console::puts(a);
   // Console::puts("\n");
   // check if the pte is valid and writable
   if ((page_table[pte_index] & 1) == 0)
   {
      // Console::puts("Page Table Entry not valid\n");
      // map a valid frame for this pte
      unsigned long frame =  process_mem_pool->get_frames(1) * Machine::PAGE_SIZE;
      // i_to_a(frame, a);
      // Console::puts("Mapped frame at ");
      // Console::puts(a);
      // Console::puts("\n");
      page_table[pte_index] = frame | 3;
      // assert(false);
   }else if ((page_table[pte_index] & 2) == 0)
   {
      Console::puts("Page Table Entry not writable\n");
      assert(false);
   }

   // flush the tlb
   // write_cr3((unsigned long) current_page_table->page_directory);
   // Console::puts("Page Fault Handled\n");

}

