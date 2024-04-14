/*
     File        : blocking_disk.c

     Author      : 
     Modified    : 

     Description : 

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "utils.H"
#include "console.H"
#include "blocking_disk.H"
#include "machine.H"
#include "thread.H"
#include "scheduler.H"

extern Scheduler *SYSTEM_SCHEDULER;

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

BlockingDisk::BlockingDisk(DISK_ID _disk_id, unsigned int _size) 
  : SimpleDisk(_disk_id, _size) {
    disk_queue = new Queue();
}

/*--------------------------------------------------------------------------*/
/* SIMPLE_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/

void BlockingDisk::read(unsigned long _block_no, unsigned char * _buf) {
  if(Machine::interrupts_enabled()) Machine::disable_interrupts();
  SimpleDisk::read(_block_no, _buf);
  // if (!Machine::interrupts_enabled())
  //   Machine::enable_interrupts();
}


void BlockingDisk::write(unsigned long _block_no, unsigned char * _buf) {
  if(Machine::interrupts_enabled()) Machine::disable_interrupts();
  SimpleDisk::write(_block_no, _buf);
  // if(!Machine::interrupts_enabled()) 
  //   Machine::enable_interrupts();
}

void BlockingDisk::wait_until_ready(){
  // Console::puts("wait until ready\n");
  while(!SimpleDisk::is_ready()){
    assert(disk_queue->get_length()==0); // for single thread
    disk_queue->add(Thread::CurrentThread());
    SYSTEM_SCHEDULER->yield();
  }
}

bool BlockingDisk::is_ready(){
  Console::puts("Is ready from inside blocking disk\n");
  return SimpleDisk::is_ready();
}