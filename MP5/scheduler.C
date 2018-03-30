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

Scheduler::Scheduler() {
  // initialize the queue head/tail pointer
  head = NULL;
  tail = NULL;
  Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {
  	// get the tail of the ready queue
  	ThreadQueueNode *n;
  	if(tail == NULL)
  		assert(flase);
  	else
  	{
  		n = tail;

  	}
}

void Scheduler::resume(Thread * _thread) {
  	add(_thread);
}

void Scheduler::add(Thread * _thread) {
  	// allocate a queue node
	ThreadQueueNode *n = new ThreadQueueNode;
	n->_thread = _thread;
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
  assert(false);
}
