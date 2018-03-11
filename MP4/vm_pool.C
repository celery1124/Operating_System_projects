/*
 File: vm_pool.C
 
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

#include "machine.H"
#include "vm_pool.H"
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
/* METHODS FOR CLASS   V M P o o l */
/*--------------------------------------------------------------------------*/

VMPool::VMPool(unsigned long  _base_address,
               unsigned long  _size,
               ContFramePool *_frame_pool,
               PageTable     *_page_table) {
    base_address = _base_address;
    size = _size;

    // nitial allocate pointer and region number, first page sotre metadata
    alloc_pointer = base_address + PAGE_SIZE;
    region_no = 0;
    region_list = base_address;

    // register to page table
    next = NULL;
    _page_table->register_pool(this);
    Console::puts("Constructed VMPool object.\n");
}

unsigned long VMPool::allocate(unsigned long _size) {
    unsigned long ret;
    // sanctity check
    if(_size == 0)
    {
        Console::puts("Allocate size 0, return NULL\n");
        return 0;
    }

    // round to multiple of PAGE_SIZE
    unsigned long size = _size;
    if(_size % PAGE_SIZE != 0)
    {
        size = (_size / PAGE_SIZE + 1) * PAGE_SIZE;
    }

    // add to region_list and allocate memory 
    region_list[region_no].start_addr = alloc_pointer;
    region_list[region_no].size = size;
    region_no++;
    ret = alloc_pointer;
    alloc_pointer += size;
    return ret;

    Console::puts("Allocated region of memory.\n");
}

void VMPool::release(unsigned long _start_address) {
    assert(false);
    Console::puts("Released region of memory.\n");
}

bool VMPool::is_legitimate(unsigned long _address) {
    assert(false);
    Console::puts("Checked whether address is part of an allocated region.\n");
}

