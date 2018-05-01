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
    unsigned int write_cnt = 0;
    int block_index = 0;
    int block_offset = 0;
    unsigned char buf[512];
    int cnt = 0;
    // direct block
    while(write_cnt < _n && block_index < 5)
    {
        block_index = (inode.size+1) / BLOCK_SIZE;
        block_offset = (inode.size+1) % BLOCK_SIZE;
        cnt = 0;
        if(block_offset != 0)
            fs->disk->read(inode.direct_index[block_index], buf)
        for (int i = block_offset; i < BLOCK_SIZE; i++)
        {
            buf[i] = _buf[write_cnt++];
            cnt++;
            if(write_cnt == _n)
                break;
        }

        // allocate a new data block
        if(block_offset == 0)
        {
            assert(inode.direct_index[block_index] = fs->alloc_data_block());
            fs->disk->write(inode.direct_index[block_index], buf);
        }
        // update to exist block
        else
        {
            fs->disk->write(inode.direct_index[block_index], buf);
        }
        inode.size += cnt;
    }

    if (write_cnt < _n)
    {
        // indirect block
        unsigned char indirect_index_buf[512];
        uint16_t * indirect_index = (uint16_t *)indirect_index_buf;
        if(inode.indirect_index == 0)
            assert(inode.indirect_index = fs->alloc_data_block());
        else
            fs->disk->read(inode.indirect_index, indirect_index_buf);
        
        while(write_cnt < _n)
        {
            block_index = (inode.size+1) / BLOCK_SIZE - 5;
            if (block_index > BLOCK_SIZE/sizeof(uint16_t))
            {
                Console::puts("Beyond Maximum size of a file\n");
                assert(false);
            }
            block_offset = (inode.size+1) % BLOCK_SIZE;
            cnt = 0;
            if(block_offset != 0)
                fs->disk->read(indirect_index[block_index], buf)
            for (int i = block_offset; i < BLOCK_SIZE; i++)
            {
                buf[i] = _buf[write_cnt++];
                cnt++;
                if(write_cnt == _n)
                    break;
            }

            // allocate a new data block
            if(block_offset == 0)
            {
                assert(indirect_index[block_index] = fs->alloc_data_block());
                fs->disk->write(indirect_index[block_index], buf);
            }
            // update to exist block
            else
            {
                fs->disk->write(indirect_index[block_index], buf);
            }
            inode.size += cnt;
        }
    }

    // flush the inode
    fs->flush_inode(inode_id, inode);
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
