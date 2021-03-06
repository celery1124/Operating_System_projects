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
#include "blocking_disk.H"
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
  disklist_head = NULL;
  Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {
    // first we try to clean the terminated threads
    ThreadCleanNode *clean_c = cleanup_head;
    ThreadCleanNode *clean_p = NULL;
    while(clean_c != NULL)
    {
        if(clean_c->thread != current_thread)
        {
            // clean up the thread
            delete clean_c->thread;
            Console::puts("Thread: "); Console::puti(Thread::CurrentThread()->ThreadId()); Console::puts(" terminated\n");
            if(clean_p != NULL)
            {
                clean_p->next = clean_c->next;
                delete clean_c;
            }
            else
            {
                delete clean_c;
                cleanup_head = NULL;
            }
        }
        clean_p = clean_c;
        clean_c = clean_c->next;
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
      tail = tail->prev;
  	}
  	thread_to_go = n->thread;
  	delete n;
    //Console::puts("resume thread "); Console::puti((int)thread_to_go);Console::puts(" \n");
  	Thread::dispatch_to(thread_to_go);
}

void Scheduler::resume(Thread * _thread) {
    // first check disklist to see whether disk request threads are ready
    DiskList *disk_inst = disklist_head;
    BlockingDisk *disk;
    while(disk_inst != NULL)
    {   
        disk = disk_inst->disk;
        Thread *thread = disk->get_head_thread();
        if(thread != NULL)
        {
            bool flag = disk->is_ready() || disk->disk_status;
            //Console::puts("check is ready() "); Console::puti(flag);Console::puts(" \n");
            if(flag)
            {
              add(thread);
            }

        }

        disk_inst = disk_inst->next;
    }
    // add current context thread 
  	add(_thread);
}

void Scheduler::add(Thread * _thread) {
  //Console::puts("add thread "); Console::puti((int)_thread);Console::puts(" \n");
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

void Scheduler::disk_register(BlockingDisk * _disk) {
      DiskList *disk_inst = new DiskList;
      disk_inst->disk = _disk;
      disk_inst->next = NULL;
      if (disklist_head == NULL)
      {
          disklist_head = disk_inst;
      }
      else
      {
          disk_inst->next = disklist_head;
          disklist_head = disk_inst;
      }

   }
