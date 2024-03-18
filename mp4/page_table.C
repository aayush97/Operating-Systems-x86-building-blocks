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
VMPool * PageTable::registered_pools_ll = NULL;

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
   page_directory = (unsigned long *)(process_mem_pool->get_frames(1) * Machine::PAGE_SIZE);
   // Map the first 4MB of memory to the same physical addresses
   unsigned long *page_table = (unsigned long *)(process_mem_pool->get_frames(1) * Machine::PAGE_SIZE);
   unsigned long * page_directory_v = PDE_address(page_directory);
   page_directory_v[0] = ((unsigned long)page_table) | 3;
   unsigned long * page_table_v = PTE_address(page_directory, 0);
   for (unsigned int i = 0; i < Machine::PT_ENTRIES_PER_PAGE; i++)
   {
      page_table_v[i] = (i << 12) | 3;
   }
   // unused pages
   for (unsigned int i = 1; i < Machine::PT_ENTRIES_PER_PAGE; i++)
   {
      page_directory_v[i] = 0 | 2;
   }
   // map the last page directory entry to point to the page directory table itself
   page_directory_v[Machine::PT_ENTRIES_PER_PAGE - 1] = (unsigned long) page_directory | 3; 
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
   write_cr0(read_cr0() | 0x80000000);
   paging_enabled = 1;
   Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{
   // read the offending address
   unsigned long offending_addr = read_cr2();

   // check if the offending address is legitimate
   VMPool *head = registered_pools_ll;
   while(head != NULL){
      if (head->is_legitimate(offending_addr)) break;
      head = head->next;
   }
   
   if(head == NULL && registered_pools_ll != NULL){
      Console::puts("Not a legitimate address");
      assert(false);
   }

   unsigned int pde_index = (offending_addr >> 22);
   unsigned int pte_index = (offending_addr >> 12) & 0x3FF;
   unsigned long * page_directory = PDE_address(current_page_table->page_directory);
   unsigned long * page_table;

   // check if the pde is valid and writable
   if ((page_directory[pde_index] & 1) == 0)
   {
      // generate a valid page table for this pde
      page_table = (unsigned long *)(process_mem_pool->get_frames(1) * Machine::PAGE_SIZE);
      page_directory[pde_index] = ((unsigned long)page_table) | 3;
      // convert into logical address
      page_table = PTE_address(current_page_table->page_directory ,pde_index);

      for (unsigned int i = 0; i < Machine::PT_ENTRIES_PER_PAGE; i++)
      {
         page_table[i] = 0 | 2;
      }
   }else if((page_directory[pde_index] & 2) == 0)
   {
      Console::puts("Page Directory Entry not writable\n");
      assert(false);
   }else{
      page_table = PTE_address(current_page_table->page_directory ,pde_index);
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

unsigned long *PageTable::PDE_address(unsigned long *p_page_dir)
{
   if(paging_enabled == 0) return p_page_dir;
   unsigned long v_addr = 0xFFFFF000;
   return (unsigned long *) v_addr;
}

unsigned long *PageTable::PTE_address(unsigned long *p_page_dir, int pde_index)
{
   if(paging_enabled == 0) return (unsigned long *) (p_page_dir[pde_index] & 0xFFFFF000);
   unsigned long v_addr = 0xFFC00000 | (pde_index) << 12;
   return (unsigned long *) v_addr;
}

void PageTable::register_pool(VMPool * _vm_pool){
   if (registered_pools_ll == NULL){
      registered_pools_ll = _vm_pool;
   }else{
      _vm_pool->next = registered_pools_ll;
      registered_pools_ll->prev = _vm_pool;
      registered_pools_ll = _vm_pool;
   }
}

void PageTable::free_page(unsigned long page_no){
   unsigned int pte_index = page_no & 0x00003FF;
   unsigned int pde_index = page_no >> 10;
   unsigned long *page_directory_v = PDE_address(page_directory);
   // check if page directory entry is valid
   if (page_directory_v[pde_index] & 1){
      unsigned long* page_table_v = PTE_address(page_directory, pde_index);
      // check if page table entry is valid
      if(page_table_v[pte_index] & 1){
         unsigned long frame_no = page_table_v[pte_index] >> 12;
         page_table_v[pte_index] &= 0xFFFFFFFE;
         process_mem_pool->release_frames(frame_no);
      }
   }

}