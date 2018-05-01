/*
     File        : file.C

     Author      : Riccardo Bettati
     Modified    : 2017/05/01

     Description : Implementation of simple File class, with support for
                   sequential read/write operations.
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
#include "file.H"

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

File::File(FileSystem *_fs) {
    /* We will need some arguments for the constructor, maybe pointer to disk
     block with file management and allocation data. */
    curr_pointer = 0;
    fs = _fs;
}

/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

int File::Read(unsigned int _n, char * _buf) {
    Console::puts("reading from file\n");
    assert(false);
}


void File::Write(unsigned int _n, const char * _buf) {
    Console::puts("writing to file\n");
    assert(false);
}

void File::Reset() {
    curr_pointer = 0;
}

void File::Rewrite() {
    //release all the data blocks
    inode.size = 0;
    curr_pointer = 0;
    // direct index
    for (int i = 0; i < 5; i++)
    {
        if(inode.direct_index[i] != 0)
            assert(fs->release_data_block(inode.direct_index[i]));
    }
    // indirect index
    if (inode.indirect_index != 0)
    {
        unsigned char buf[512];
        fs->disk->read(inode.indirect_index, buf);
        uint16_t *block_index = (uint16_t *)buf;
        for(int i = 0; i < BLOCK_SIZE/sizeof(uint16_t); i++)
        {
            if (block_index[i] != 0)
                assert(fs->release_data_block(block_index[i]));
        }

    }
}


bool File::EoF() {
    return curr_pointer == inode.size;
}
