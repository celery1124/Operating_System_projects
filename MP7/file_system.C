/*
     File        : file_system.C

     Author      : Riccardo Bettati
     Modified    : 2017/05/01

     Description : Implementation of simple File System class.
                   Has support for numerical file identifiers.
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file_system.H"


/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

FileSystem::FileSystem() {
    disk = NULL;
}

/*--------------------------------------------------------------------------*/
/* FILE SYSTEM FUNCTIONS */
/*--------------------------------------------------------------------------*/

bool FileSystem::Mount(SimpleDisk * _disk) {
    if(disk != NULL)
    {
        Console::puts("Already mounted\n");
        return false;
    }
    else
    {
        disk = _disk;
        return true
    }
}

bool FileSystem::Format(SimpleDisk * _disk, unsigned int _size) {
    // write buffer
    unsigned char buf[512];
    // 1, write the super block
    *(unsigned int *)buf = _size;
    _disk->write(0, buf);

    // 2, wipe the file table;
    for (int i=0;i<512;i++)
        buf[i] = 0;
    _disk->write(1, buf);

    // 2, initialize inode bitmap
    for (int i=0;i<64;i++)
        buf[i] = 1;
    _disk->write(2, buf);

    // 3, initialize data block bitmap, need a little bit math
    int data_block_num = _size / BLOCK_SIZE;
    int db_bitmap_block_num;
    if (data_block_num % (BLOCK_SIZE * 8) == 0)
        db_bitmap_block_num = data_block_num / (BLOCK_SIZE * 8);
    else
        db_bitmap_block_num = data_block_num / (BLOCK_SIZE * 8) + 1;
    int remain_bytes = data_block_num/8;
    for (int i=0; i<db_bitmap_block_num; i++)
    {
        int bytes_in_block = BLOCK_SIZE;
        if (i == db_bitmap_block_num - 1)
            bytes_in_block = remain_bytes;
        for(int j=0; j=bytes_in_block; j++)
        {
            buf[i] = 0xFF;
        }
        _disk->write(2, buf);
        remain_bytes -= BLOCK_SIZE;
    }

    Console::puts("Format complete\n");    
}

File * FileSystem::LookupFile(int _file_id) {
    Console::puts("looking up file\n");
    assert(false);
}

bool FileSystem::CreateFile(int _file_id) {
    Console::puts("creating file\n");
    assert(false);
}

bool FileSystem::DeleteFile(int _file_id) {
    Console::puts("deleting file\n");
    assert(false);
}
