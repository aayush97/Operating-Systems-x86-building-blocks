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

void Queue::print_queue()
{
  QueueNode *current = head;
  while (current != NULL)
  {
    Console::putui((unsigned int) current->thread);
    Console::puts(" ");
    current = current->next;
  }
  Console::puts("\n");
}

Queue::Queue(){
  head = NULL;
  tail = NULL;
  length = 0;
}

int Queue::get_length(){
  return length;
}

Thread* Queue::pop(){
  if (head==NULL){
    assert(length == 0);
    return NULL;
  }
  QueueNode* popped_node = head;
  head = popped_node->next;
  if (head==NULL){
    tail = NULL;
  }
  Thread* popped_thread = popped_node->thread;
  delete popped_node;
  length--;
  return popped_thread;
}

void Queue::add(Thread * thread){
  QueueNode* new_node = new QueueNode(thread, NULL);
  if (head==NULL){
    head = new_node;
    tail = new_node;
  }else{
    tail->next = new_node;
    tail = new_node;
  }
  length++;
}

bool Queue::delete_elem(Thread * thread){
  QueueNode* current = head;
  QueueNode* previous = NULL;
  while(current != NULL){
    if (current->thread->ThreadId() == thread->ThreadId()){
      if(previous != NULL){ // deleted node not the head
        previous->next = current->next;
        if (tail->thread->ThreadId() == thread->ThreadId()){ // deleted node is the last node
          tail = previous;
          delete current;
          length--;
          return true;
        }
      }else{ // deleted node is the head
        head = current->next;
        if (tail->thread->ThreadId() == thread->ThreadId()){// only one node in the queue
          tail = NULL;
          delete current;
          length--;
          return true;
        }
      }
    }
    previous = current;
    current = current->next;
  }
  return false;
}

Scheduler::Scheduler() {
  ready_queue = new Queue();
  Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {
  Console::puts("Yielding.\n");
  if(Machine::interrupts_enabled()){
    Machine::disable_interrupts();
  }

  Thread* next_thread = ready_queue->pop();
  if (next_thread == NULL) {
    assert(ready_queue->get_length() == 0);
    Console::puts("No thread to run.\n");
  }
  Thread::dispatch_to(next_thread);
  if(!Machine::interrupts_enabled()){
    Machine::enable_interrupts();
  }
}

void Scheduler::resume(Thread * _thread) {
  if (Machine::interrupts_enabled())
    Machine::disable_interrupts();
  Console::puts("Resuming.\n");
  assert(_thread != NULL);
  ready_queue->add(_thread);
  // if(!Machine::interrupts_enabled())
  //   Machine::enable_interrupts();
}

void Scheduler::add(Thread * _thread) {
  resume(_thread);
}

void Scheduler::terminate(Thread * _thread) {
  if(Thread::CurrentThread()->ThreadId() == _thread->ThreadId()){
    ready_queue->delete_elem(_thread);
    return;
  }
  assert(false);
}

void RRScheduler::yield() {
  Machine::outportb(0x20, 0x20);
  Scheduler::yield();
}
