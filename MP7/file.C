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
#include "file_system.H"

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
    unsigned int read_cnt = 0;
    int block_index = 0;
    int block_offset = 0;
    unsigned char buf[512];

    // direct block
    while(read_cnt < _n)
    {
        block_index = curr_pointer / BLOCK_SIZE;
        block_offset = curr_pointer % BLOCK_SIZE;
        
        if(block_index >= 5)
            break;
        
        fs->disk->read(inode.direct_index[block_index], buf);
        Console::puts("read block: ") ;Console::puti(inode.direct_index[block_index]);Console::puts("\n")  ;  
        for (int i = block_offset; i < BLOCK_SIZE; i++)
        {
            _buf[read_cnt++] = buf[i];
            curr_pointer++;
            if(read_cnt == _n || EoF())
                return read_cnt;
        }
    }

    // indirect block
    unsigned char indirect_index_buf[512];
    uint16_t * indirect_index = (uint16_t *)indirect_index_buf;
    fs->disk->read(inode.indirect_index, indirect_index_buf);
    
    while(read_cnt < _n)
    {
        block_index = curr_pointer / BLOCK_SIZE - 5;
        block_offset = curr_pointer % BLOCK_SIZE;

        fs->disk->read(indirect_index[block_index], buf);
        for (int i = block_offset; i < BLOCK_SIZE; i++)
        {
            _buf[read_cnt++] = buf[i];
            curr_pointer++;
            if(read_cnt == _n || EoF())
                return read_cnt;
        }
    }
}


void File::Write(unsigned int _n, const char * _buf) {
    unsigned int write_cnt = 0;
    int block_index = 0;
    int block_offset = 0;
    unsigned char buf[512];
    int curr_block;

    // direct block
    while(write_cnt < _n)
    {
        block_index = (curr_pointer + 1) / BLOCK_SIZE;
        if(block_index >= 5)
            break;
        block_offset = (curr_pointer + 1) % BLOCK_SIZE;

        curr_block = inode.size / BLOCK_SIZE;
        if (inode.size % BLOCK_SIZE == 0)
            curr_block--;

        if(block_index <= curr_block)
            fs->disk->read(inode.direct_index[block_index], buf);
        for (int i = block_offset; i < BLOCK_SIZE; i++)
        {
            buf[i] = _buf[write_cnt++];
            curr_pointer++;
            if(write_cnt == _n)
                break;
        }

        // allocate a new data block
        if(block_index > curr_block)
        {
            assert(inode.direct_index[block_index] = fs->alloc_data_block());
            fs->disk->write(inode.direct_index[block_index], buf);
            Console::puts("write block: ") ;Console::puti(inode.direct_index[block_index]);Console::puts("\n")  ;  
            inode.size = curr_pointer;
        }
        // update to exist block
        else
        {
            fs->disk->write(inode.direct_index[block_index], buf);
        }
    }

    if (write_cnt < _n)
    {
        // indirect block
        unsigned char indirect_index_buf[512];
        uint16_t * indirect_index = (uint16_t *)indirect_index_buf;
        if(inode.indirect_index == 0)
        {
            assert(inode.indirect_index = fs->alloc_data_block());
        }
        else
            fs->disk->read(inode.indirect_index, indirect_index_buf);
        
        while(write_cnt < _n)
        {
            block_index = (curr_pointer + 1) / BLOCK_SIZE - 5;
            if (block_index > BLOCK_SIZE/sizeof(uint16_t))
            {
                Console::puts("Beyond Maximum size of a file\n");
                assert(false);
            }
            block_offset = (curr_pointer + 1) % BLOCK_SIZE;
            curr_block = inode.size / BLOCK_SIZE;
            if (inode.size % BLOCK_SIZE == 0)
                curr_block--;
            
            if(block_index <= (curr_block - 5))
                fs->disk->read(indirect_index[block_index], buf);
            for (int i = block_offset; i < BLOCK_SIZE; i++)
            {
                buf[i] = _buf[write_cnt++];
                curr_pointer++;
                if(write_cnt == _n)
                    break;
            }

            // allocate a new data block
            if(block_index > (curr_block - 5))
            {
                assert(indirect_index[block_index] = fs->alloc_data_block());
                fs->disk->write(indirect_index[block_index], buf);
                inode.size = curr_pointer;
            }
            // update to exist block
            else
            {
                fs->disk->write(indirect_index[block_index], buf);
            }
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
