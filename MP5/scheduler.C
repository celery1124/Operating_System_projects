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
  Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {
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
  // clean up the thread
  delete _thread;
  yield();
}
