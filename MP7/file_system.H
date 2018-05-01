/* 
    File: file_system.H

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 10/04/05

    Description: Simple File System.
    

*/

#ifndef _FILE_SYSTEM_H_                   // include file only once
#define _FILE_SYSTEM_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

#define BLOCK_SIZE 512

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "file.H"
#include "simple_disk.H"
#include "frame_pool.H"      /* MEMORY MANAGEMENT */
#include "mem_pool.H"


/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARD DECLARATIONS */ 
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* F i l e S y s t e m  */
/*--------------------------------------------------------------------------*/

/* file system layout *
/*--------------------------------------------------------------------------*/
/* superblock - 0 | file table - 1 | inode bytemap - 2 | inode table - 3,4 | data block bytemap | data block */
/*--------------------------------------------------------------------------*/
class FileList {
public:
    unsigned int filename;
    unsigned int inode_num;
};

class FileSystem {

friend class File; /* -- not sure if we need this; feel free to delete */

private:
     /* -- DEFINE YOUR FILE SYSTEM DATA STRUCTURES HERE. */
     
     SimpleDisk * disk;
     unsigned int size;
     FileList file_table[64];
     unsigned char inode_bitmap[64];
     unsigned char *data_block_bitmap;
     unsigned int data_block_bitmap_size;
     unsigned int data_block_bitmap_block_size;

     uint16_t alloc_inode();
     /* Allocate Inode for a file, return 255 if no available inode */

     bool release_inode(uint16_t inode_id);
     /* Release Inode to filesystem */

     uint16_t alloc_data_block();
     /* Allocate free data block return 0 if no available data block*/

     bool release_data_block(uint16_t block_addr);
     /* Release data block */
     
public:

    FileSystem();
    ~FileSystem(){
        if (data_block_bitmap != NULL)
            delete [] data_block_bitmap;
    };
    /* Just initializes local data structures. Does not connect to disk yet. */
    
    bool Mount(SimpleDisk * _disk);
    /* Associates this file system with a disk. Limit to at most one file system per disk.
     Returns true if operation successful (i.e. there is indeed a file system on the disk.) */
    
    static bool Format(SimpleDisk * _disk, unsigned int _size);
    /* Wipes any file system from the disk and installs an empty file system of given size. */
    
    File * LookupFile(int _file_id);
    /* Find file with given id in file system. If found, return the initialized
     file object. Otherwise, return null. */
    
    bool CreateFile(int _file_id);
    /* Create file with given id in the file system. If file exists already,
     abort and return false. Otherwise, return true. */
    
    bool DeleteFile(int _file_id);
    /* Delete file with given id in the file system; free any disk block occupied by the file. */
   
};
#endif
