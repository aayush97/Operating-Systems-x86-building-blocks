/*
 File: scheduler.C
 
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

#include "scheduler.H"
#include "thread.H"
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
/* METHODS FOR CLASS   S c h e d u l e r  */
/*--------------------------------------------------------------------------*/

FIFOQueue::FIFOQueue(){
  queue_head = NULL;
  queue_tail = NULL;
}

Thread* FIFOQueue::pop(){
  Thread* popped_thread = queue_head;
  queue_head = (Thread *) queue_head->getCargo();
  if (queue_head==NULL){
    queue_tail = NULL;
  }
  popped_thread->setCargo(NULL);
  return popped_thread;
}

void FIFOQueue::add(Thread * thread){
  if (queue_head==NULL){
    queue_head = thread;
    queue_tail = thread;
    thread->setCargo(NULL);
  }else{
    queue_tail->setCargo(thread);
    thread->setCargo(NULL);
  }
}

Scheduler::Scheduler() {
  queue = FIFOQueue();
  cur_thread = NULL;
  Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {
  Thread* next_thread = queue.pop();
  // move the cur_thread to the end of the linked list
  queue.add(cur_thread);
  cur_thread = next_thread;
  Thread::dispatch_to(cur_thread);
}

void Scheduler::resume(Thread * _thread) {
  queue.add(_thread);
}

void Scheduler::add(Thread * _thread) {
  resume(_thread);
}

void Scheduler::terminate(Thread * _thread) {
  
}
