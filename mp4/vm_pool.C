/*
 File: vm_pool.C
 
 Author:
 Date  :
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "vm_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   V M P o o l */
/*--------------------------------------------------------------------------*/

VMPool::VMPool(unsigned long  _base_address,
               unsigned long  _size,
               ContFramePool *_frame_pool,
               PageTable     *_page_table) {
    page_table = _page_table;
    base_address = _base_address;
    size = _size;
    frame_pool = _frame_pool;
    next = NULL;
    prev = NULL;
    page_table->register_pool(this);

    // allocate from the pool itself
    free_regions = (Region *) (base_address);
    allocated_regions = (Region *)(base_address + Machine::PAGE_SIZE);
    allocated_regions[0].base_page_no = (base_address / Machine::PAGE_SIZE);
    allocated_regions[0].num_pages = 1;
    allocated_regions[1].base_page_no = (base_address / Machine::PAGE_SIZE) + 1;
    allocated_regions[1].num_pages = 1;
    num_allocated_regions = 2;
    free_regions[0].base_page_no = (base_address / Machine::PAGE_SIZE) + 2;
    free_regions[0].num_pages = (size / Machine::PAGE_SIZE) - 2;
    
    num_free_regions = 1;
    
    Console::puts("Constructed VMPool object.\n");
}

unsigned long VMPool::allocate(unsigned long _size) {
    unsigned long required_num_pages = (_size + Machine::PAGE_SIZE - 1) / Machine::PAGE_SIZE;
    for(int i=0; i<num_free_regions; i++){
        if(free_regions[i].num_pages > required_num_pages){
            free_regions[i].num_pages -= required_num_pages;
            allocated_regions[num_allocated_regions].base_page_no = free_regions[i].base_page_no + free_regions[i].num_pages - required_num_pages;
            allocated_regions[num_allocated_regions].num_pages = required_num_pages;
            num_allocated_regions++;
            break;
        }
    }
    Console::puts("Allocated region of memory.\n");
    return allocated_regions[num_allocated_regions - 1].base_page_no * Machine::PAGE_SIZE;
}

void VMPool::release(unsigned long _start_address) {
    unsigned long release_base_page_no = _start_address / Machine::PAGE_SIZE;
    for(int i=0; i<num_allocated_regions;i++){
        if(allocated_regions[i].base_page_no == release_base_page_no){
            free_regions[num_free_regions].base_page_no = allocated_regions[i].base_page_no;
            free_regions[num_allocated_regions].num_pages = allocated_regions[i].num_pages;
            for(int j=0;j<allocated_regions[i].num_pages;j++){
                page_table->free_page(allocated_regions[i].base_page_no + j);
            }
            for(int j=i+1;j<num_allocated_regions;j++){
                allocated_regions[j-1].base_page_no = allocated_regions[j].base_page_no;
                allocated_regions[j-1].num_pages = allocated_regions[j].num_pages;
            }
            num_allocated_regions--;
            num_free_regions++;
            break;
        }
    }
    Console::puts("Released region of memory.\n");
}


bool VMPool::is_legitimate(unsigned long _address) {
    // if the address is in the first two pages of the pool
    if (_address >= base_address && _address < base_address + 2*Machine::PAGE_SIZE) return true;
    for (int i=0; i<num_allocated_regions; i++){
        if((_address >= allocated_regions[i].base_page_no * Machine::PAGE_SIZE) && (_address < (allocated_regions[i].base_page_no + allocated_regions[i].num_pages) * Machine::PAGE_SIZE)){
            return true;
        }
    }
    return false;
}

