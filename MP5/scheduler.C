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

extern Thread * current_thread;

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   S c h e d u l e r  */
/*--------------------------------------------------------------------------*/

Scheduler::Scheduler() {
  // initialize the queue head/tail pointer
  head = NULL;
  tail = NULL;
  cleanup_head = NULL;
  Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {
    // first we try to clean the terminated threads
    ThreadCleanNode *clean_n = cleanup_head;
    while(clean_n != NULL)
    {
        delete clean_n->thread;
        clean_n = clean_n->next;
    }

  	// get the tail of the ready queue
  	ThreadQueueNode *n;
  	Thread *thread_to_go;
  	if (tail == NULL) {
  		assert(false);
  	}
  	else
  	{
  		n = tail;
  		tail->prev->next = NULL;
  	}
  	thread_to_go = n->thread;
  	current_thread = thread_to_go;
  	delete n;
  	Thread::dispatch_to(thread_to_go);
}

void Scheduler::resume(Thread * _thread) {
  	add(_thread);
}

void Scheduler::add(Thread * _thread) {
  assert(_thread);
  // reguster the shceduler for thread (only need to register once)
  if(Thread::sched == NULL)
    Thread::register_scheduler(this);
  // allocate a queue node
	ThreadQueueNode *n = new ThreadQueueNode;
	n->thread = _thread;
	n->next = NULL;
	n->prev = NULL;
	// add the node to head
	if(head == NULL && tail == NULL)
	{
		head = n;
		tail = n;
	}
	else if(head != NULL && tail != NULL)
	{
		n->next = head;
		head->prev = n;
		head = n;
	}
	else
		assert(false);
}

void Scheduler::terminate(Thread * _thread) {
  // add the thread to clean up queue
  ThreadCleanNode *n = new ThreadCleanNode;
  n->thread = _thread;
  n->next = NULL;
  if(cleanup_head == NULL)
  {
      cleanup_head = n;
  }
  else
  {
      n->next = cleanup_head;
      cleanup_head = n;
  }
  yield();
}
