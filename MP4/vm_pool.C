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

    // initialize occupy region list
    occupy_region_no = 0;
    occupy_region_list = (Region *)base_address;

    frame_pool = _frame_pool;
    page_table = _page_table;

    // register to page table
    next = NULL;
    page_table->register_pool(this);

    // nitial allocate pointer and region number, first page sotre metadata
    free_region_no = 1;
    free_region_list = (Region *)(base_address + PAGE_SIZE);
    free_region_list[0].start_addr = base_address + PAGE_SIZE * 2;
    free_region_list[0].size = size - PAGE_SIZE * 2;
    Console::puts("Constructed VMPool object.\n");
}

unsigned long VMPool::allocate(unsigned long _size) {
    unsigned long alloc_pointer = 0;
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

    // find a region on free region list
    for(int i = 0; i < free_region_no; i++)
    {
        if(free_region_list[i].size >= size)
        {
            alloc_pointer = free_region_list[i].start_addr;
            // adjust free region list
            free_region_list[i].start_addr += size;
            free_region_list[i].size -= size;
            break;
        }
    }
    if(alloc_pointer == 0)
    {
        Console::puts("Out of virtual memory space...\n");
        assert(false);
    }

    // add to occupy region_list 
    occupy_region_list[occupy_region_no].start_addr = alloc_pointer;
    occupy_region_list[occupy_region_no].size = size;
    occupy_region_no++;
    ret = alloc_pointer;
    return ret;

    Console::puts("Allocated region of memory.\n");
}

void VMPool::release(unsigned long _start_address) {
    unsigned long page_no;
    unsigned long release_size;
    // scan the whole region list
    for(int i = 0; i < occupy_region_no; i++)
    {
        if(_start_address == occupy_region_list[i].start_addr)
        {
            release_size = occupy_region_list[i].size;
            // free page one by one
            for(int j = 0; j < release_size/PAGE_SIZE; j++)
            {
                page_no = occupy_region_list[i].start_addr/PAGE_SIZE + j;
                page_table->free_page(page_no);
            }
            // swap the end entry of the occupy region list
            occupy_region_list[i].start_addr = occupy_region_list[occupy_region_no-1].start_addr;
            occupy_region_list[i].size = occupy_region_list[occupy_region_no-1].size;
            occupy_region_no--;

            // coalesce the release region to free region list
            int k;
            for(k = 0; k < free_region_no; k++)
            {
                if(_start_address + release_size == free_region_list[k].start_addr)
                {
                    free_region_list[k].start_addr -= release_size;
                    free_region_list[k].size += release_size;
                    break;
                }
            }
            // cannot coalesce, add a new free region
            if(k == free_region_no)
            {
                free_region_list[free_region_no].start_addr = _start_address;
                free_region_list[free_region_no].size = release_size;
                free_region_no++;
            }

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

