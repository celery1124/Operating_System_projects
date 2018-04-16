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

#include "thread.H"         /* THREAD MANAGEMENT */
#include "scheduler.H"

extern Scheduler * SYSTEM_SCHEDULER;

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

BlockingDisk::BlockingDisk(DISK_ID _disk_id, unsigned int _size) 
  : SimpleDisk(_disk_id, _size) {
    SYSTEM_SCHEDULER->disk_register(this);
    head = NULL;
    tail = NULL;
}

/*--------------------------------------------------------------------------*/
/* SIMPLE_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/

void req_enqueue() {
  ReqQueueNode *req = new ReqQueueNode;
  req->thread = Thread::CurrentThread();
  req->next = NULL;
  if (tail == NULL)
  {
      head = req;
      tail = req;
  }
  else
  {
      tail->next = req;
      tail = req;
  }
}

void req_dequeue() {
  ReqQueueNode *req = head;
  head = head->next;
  if(head == NULL)
    tail = NULL;
  delete req;
}

void BlockingDisk::read(unsigned long _block_no, unsigned char * _buf) {
  // -- REPLACE THIS!!!
  
  // first add to disk queue (tail)
  req_enqueue();
  SYSTEM_SCHEDULER->yield();
  // ready to issue command
  issue_operation(READ, _block_no);
  SYSTEM_SCHEDULER->yield();

  // read data from port
  int i;
  unsigned short tmpw;
  for (i = 0; i < 256; i++) {
    tmpw = Machine::inportw(0x1F0);
    _buf[i*2]   = (unsigned char)tmpw;
    _buf[i*2+1] = (unsigned char)(tmpw >> 8);
  }

  // remove from request queue
  req_dequeue();

}


void BlockingDisk::write(unsigned long _block_no, unsigned char * _buf) {
  // -- REPLACE THIS!!!
  
}
