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
  // -- REPLACE THIS!!!
  if(Machine::interrupts_enabled()) Machine::disable_interrupts();
  SimpleDisk::read(_block_no, _buf);
  Machine::enable_interrupts();

}


void BlockingDisk::write(unsigned long _block_no, unsigned char * _buf) {
  // -- REPLACE THIS!!!
  if(Machine::interrupts_enabled()) Machine::disable_interrupts();
  SimpleDisk::write(_block_no, _buf);
  Machine::enable_interrupts();
}

void BlockingDisk::wait_until_ready(){
  if(!SimpleDisk::is_ready()){
    disk_queue->add(Thread::CurrentThread());
    SYSTEM_SCHEDULER->yield();
  }
}