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
#include "vm_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"
#include "page_table.H"

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
    region_list = (Region *)base_address;
    frame_pool = _frame_pool;
    page_table = _page_table;

    // register to page table
    next = NULL;
    page_table->register_pool(this);
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
    unsigned long page_no;
    // scan the whole region list
    for(int i = 0; i < region_no; i++)
    {
        if(_start_address == region_list[i].start_addr)
        {
            // free page one by one
            for(int j = 0; j < region_list[i].size/PAGE_SIZE; j++)
            {
                page_no = region_list[i].start_addr/PAGE_SIZE + j;
                page_table->free_page(page_no);
            }
            // swap the end entry of the region list
            region_list[i].start_addr = region_list[region_no-1].start_addr;
            region_list[i].size = region_list[region_no-1].size;
            region_no--;
            Console::puts("Released region of memory.\n");
            return;
        }
    }
    // _start_address not in the region list
    Console::puts("Released address not in the allocated Region.\n");
}

bool VMPool::is_legitimate(unsigned long _address) {
    if(_address >= base_address && _address < base_address + PAGE_SIZE)
        return true;

    unsigned long start_addr, end_addr;
    for(int i=0; i < region_no; i++)
    {
        start_addr = region_list[i].start_addr;
        end_addr = region_list[i].start_addr + region_list[i].size;
        if(_address >= start_addr && _address < end_addr)
            return true;
    }
    return false;
    Console::puts("Checked whether address is part of an allocated region.\n");
}

