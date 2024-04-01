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

void i_to_a(unsigned long i, char *a)
{
  int j = 0;
  while (i > 0)
  {
    a[j] = (i % 10) + '0';
    i = i / 10;
    j++;
  }
  a[j] = 0;
  int k = 0;
  j--;
  while (k < j)
  {
    char c = a[k];
    a[k] = a[j];
    a[j] = c;
    k++;
    j--;
  }
}

void ReadyQueue::print_queue()
{
  Thread *current = queue_head;
  while (current != NULL)
  {
    char a[10];
    i_to_a((unsigned long) current, a);
    Console::puts(a);
    Console::puts(" ");
    current = (Thread *)current->getCargo();
  }
  Console::puts("\n");
}

ReadyQueue::ReadyQueue(){
  queue_head = NULL;
  queue_tail = NULL;
  length = 0;
}


Thread* ReadyQueue::pop(){
  if (queue_head==NULL){
    char a[10];
    i_to_a((unsigned long)queue_tail, a);
    Console::puts("Queue: ");
    print_queue();
    Console::puts("\n");
    assert(length == 0);
    return NULL;
  }
  Thread* popped_thread = queue_head;
  queue_head = (Thread *) queue_head->getCargo();
  if (queue_head==NULL){
    queue_tail = NULL;
  }
  popped_thread->setCargo(NULL);
  length--;
  return popped_thread;
}

void ReadyQueue::add(Thread * thread){
  if (queue_head==NULL){
    queue_head = thread;
    queue_tail = thread;
    thread->setCargo(NULL);
  }else{
    queue_tail->setCargo(thread);
    thread->setCargo(NULL);
    queue_tail = thread;
  }
  Console::puts("Adding thread ");
  print_queue();
  length++;
}

bool ReadyQueue::delete_thread(Thread * thread){
  Thread* current = queue_head;
  Thread* previous = NULL;
  while(current != NULL){
    if (current->ThreadId() == thread->ThreadId()){
      if(previous != NULL){
        previous->setCargo((Thread *)current->getCargo());
        if (queue_tail->ThreadId() == thread->ThreadId()){
          queue_tail = previous;
          length--;
        }
      }else{
        queue_head = (Thread *)current->getCargo();
        if (queue_tail->ThreadId() == thread->ThreadId()){
          queue_tail = NULL;
          length--;
        }
      }
    }
    previous = current;
    current = (Thread *)current->getCargo();
  }
  return false;
}

Scheduler::Scheduler() {
  queue = new ReadyQueue();
  Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {
  Console::puts("Yielding.\n");
  char a[10];
  i_to_a(queue->get_length(), a);
  Console::puts("Length of ready queue: ");
  Console::puts(a);
  Console::puts("\n");
  // if(Machine::interrupts_enabled()){
  //   Machine::disable_interrupts();
  // }

  Thread* next_thread = queue->pop();
  if (next_thread == NULL) {
    assert(queue->get_length() == 0);
    Console::puts("No thread to run.\n");
  }
  i_to_a(queue->get_length(), a);
  Console::puts("Length of ready queue: ");
  Console::puts(a);
  Console::puts("\n");
  // move the cur_thread to the end of the ready queue
  // if (Thread::CurrentThread() != NULL) {
  //   queue.add(Thread::CurrentThread());
  // }
  Thread::dispatch_to(next_thread);
  // if(!Machine::interrupts_enabled()){
  //   Machine::enable_interrupts();
  // }
}

void Scheduler::resume(Thread * _thread) {
  // if (Machine::interrupts_enabled())
  //   Machine::disable_interrupts();
  Console::puts("Resuming.\n");
  assert(_thread != NULL);
  queue->add(_thread);
  char a[10];
  i_to_a(queue->get_length(), a);
  Console::puts("Length of ready queue: ");
  Console::puts(a);
  Console::puts("\n");
}

void Scheduler::add(Thread * _thread) {
  // if (Machine::interrupts_enabled())
  //   Machine::disable_interrupts();
  assert(_thread != NULL);
  resume(_thread);
}

void Scheduler::terminate(Thread * _thread) {
  if(Thread::CurrentThread()->ThreadId() == _thread->ThreadId()){
    // bool success = queue.delete_thread(_thread);
    queue->delete_thread(_thread);
    return;
  }
  assert(false);
}
