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
    data_block_bitmap = NULL;
    file_table_curr = 0;

    inode_table_start_blk = 3;
    data_block_bitmap_start_blk = 5;
}

/*--------------------------------------------------------------------------*/
/* FILE SYSTEM FUNCTIONS */
/*--------------------------------------------------------------------------*/
unsigned int FileSystem::alloc_inode(){
    unsigned int inode_id = 255;
    for (unsigned int i = 0; i< 64; i++)
    {
        if (inode_bitmap[i] == 1)
        {
            inode_id = i;
            inode_bitmap[i] = 0;
            break;
        }
    }
    // no available inode in the filesystem
    if (inode_id == 255)
        return inode_id;
    // flush inode bitmap
    unsigned char buf[BLOCK_SIZE];
    for (int i = 0; i < 64; i++)
    {
        buf[i] = inode_bitmap[i];
    }
    disk->write(2, buf);

    return inode_id;

}

bool FileSystem::release_inode(unsigned int inode_id){
    if(inode_bitmap[inode_id] != 0)
        return false;

    inode_bitmap[inode_id] = 1;
    // flush inode bitmap
    unsigned char buf[BLOCK_SIZE];
    for (int i = 0; i < 64; i++)
    {
        buf[i] = inode_bitmap[i];
    }
    disk->write(2, buf);

    // also need to clean the inode
    Inode inode;
    inode.size = 0;
    inode.indirect_index = 0;
    for (int j = 0 ; j < 5; j++)
        inode.direct_index[j] = 0;
    flush_inode(inode_id, inode);


    return true;
}

uint16_t FileSystem::alloc_data_block(){
    uint16_t block_addr = 0;

    unsigned int i = 0;
    while (data_block_bitmap[i] == 0x0 && i < data_block_bitmap_size) {
        i++;
    }
    if (i == data_block_bitmap_size)
        return block_addr;
    block_addr += i*8;

    unsigned char mask = 0x80;
    while ((mask & data_block_bitmap[i]) == 0) {
        mask = mask >> 1;
        block_addr++;
    }

    // update bitmap
    data_block_bitmap[i] ^= mask;
    // flush bitmap
    unsigned char buf[BLOCK_SIZE];
    for (int i = 0; i < data_block_bitmap_size; i++)
    {
        buf[i % BLOCK_SIZE] = data_block_bitmap[i];
        if ( (i % BLOCK_SIZE == BLOCK_SIZE - 1) || i == data_block_bitmap_size - 1 )
            disk->write(data_block_bitmap_start_blk+i, buf);
    }

    return block_addr;


}

bool FileSystem::release_data_block(uint16_t block_addr){
    unsigned int index = block_addr / 8;
    unsigned char mask = 0x80 >> (block_addr % 8);

    if((data_block_bitmap[index] & mask) != 0)
        return false;

    // update bitmap
    data_block_bitmap[index] ^= mask;
    // flush bitmap
    unsigned char buf[BLOCK_SIZE];
    for (int i = 0; i < data_block_bitmap_size; i++)
    {
        buf[i % BLOCK_SIZE] = data_block_bitmap[i];
        if ( (i % BLOCK_SIZE == BLOCK_SIZE - 1) || i == data_block_bitmap_size - 1 )
            disk->write(data_block_bitmap_start_blk+i, buf);
    }
    return true;

}

bool FileSystem::flush_inode(unsigned int inode_id, Inode inode)
{
    int inode_table_block_addr = inode_table_start_blk + inode_id / 32;
    int inode_table_index = inode_id % 32;
    unsigned char buf[BLOCK_SIZE];
    disk->read(inode_table_block_addr, buf);
    Inode *p = (Inode *)buf;
    p[inode_table_index] = inode;

    // flush
    disk->write(inode_table_block_addr, buf);

}

bool FileSystem::Mount(SimpleDisk * _disk) {
    if(disk != NULL)
    {
        Console::puts("Already mounted\n");
        return false;
    }
    disk = _disk;

    // read buffer
    unsigned char buf[BLOCK_SIZE];
    // 1, read super block
    _disk->read(0, buf);
    size = *(unsigned int *)buf;

    // 2, read file table
    _disk->read(1, buf);
    FileList *fl_p = (FileList *)buf;
    for (int i = 0; i < 64; i++)
    {
        file_table[i] = fl_p[i];
    }

    // 3, read inode bitmap
    _disk->read(2, buf);
    for (int i = 0; i < 64; i++)
    {
        inode_bitmap[i] = buf[i];
    }

    // 4, read datablock bitmap
    int block_offset = data_block_bitmap_start_blk;
    int data_block_num = size / BLOCK_SIZE;
    int bitmap_size = data_block_num / 8;
    data_block_bitmap_size = bitmap_size;
    data_block_bitmap = new unsigned char[bitmap_size];
    int db_bitmap_block_num;
    if (data_block_num % (BLOCK_SIZE * 8) == 0)
        db_bitmap_block_num = data_block_num / (BLOCK_SIZE * 8);
    else
        db_bitmap_block_num = data_block_num / (BLOCK_SIZE * 8) + 1;
    data_block_bitmap_block_size = db_bitmap_block_num;
    int byte_cnt = 0;
    for (int i = 0; i < db_bitmap_block_num; i++)
    {
        _disk->read(block_offset+i,buf);
        for (int j = 0; j < BLOCK_SIZE; j++)
        {
            data_block_bitmap[byte_cnt++] = buf[j];
            if(byte_cnt == bitmap_size)
            {
                Console::puts("Mount successfully\n");    
                return true;
            }
        }
    }

}

bool FileSystem::Format(SimpleDisk * _disk, unsigned int _size) {
    // write buffer
    unsigned char buf[BLOCK_SIZE];
    // 1, write the super block
    *(unsigned int *)buf = _size;
    _disk->write(0, buf);

    // 2, wipe the file table;
    for (int i=0;i<BLOCK_SIZE;i++)
        buf[i] = 0;
    _disk->write(1, buf);

    // 2, initialize inode bitmap
    for (int i=0;i<64;i++)
        buf[i] = 1;
    _disk->write(2, buf);

    // 3. initialize inode table
    Inode *inode_p = (Inode *)buf;
    for (int i = 0 ; i < 32; i++)
    {
        inode_p[i].size = 0;
        inode_p[i].indirect_index = 0;
        for (int j = 0 ; j < 5; j++)
            inode_p[i].direct_index[j] = 0;
    }
    _disk->write(3, buf);
    _disk->write(4, buf);

    // 4, initialize data block bitmap, need a little bit math
    int block_offset = 5;
    int data_block_num = _size / BLOCK_SIZE;
    int db_bitmap_block_num;
    if (data_block_num % (BLOCK_SIZE * 8) == 0)
        db_bitmap_block_num = data_block_num / (BLOCK_SIZE * 8);
    else
        db_bitmap_block_num = data_block_num / (BLOCK_SIZE * 8) + 1;
    int remain_bytes = data_block_num/8;
    for (int i = 0; i < db_bitmap_block_num; i++)
    {
        int bytes_in_block = BLOCK_SIZE;
        if (i == db_bitmap_block_num - 1)
            bytes_in_block = remain_bytes;
        for(int j = 0; j < bytes_in_block; j++)
        {
            buf[j] = 0xFF;
            // mark matadata block as invalid
            if( i == 0 && j <= (block_offset + db_bitmap_block_num)/8 )
                buf[j] = 0;
        }
        _disk->write(block_offset+i, buf);
        remain_bytes -= BLOCK_SIZE;
    }

    Console::puts("Format complete\n");    
    return true;
}

File * FileSystem::LookupFile(int _file_id) {
    File *ret = NULL;
    // search in the file table
    for(int i = 0; i < 64; i++)
    {
        if (file_table[i].filename == _file_id)
        {
            ret = new File(this);
            ret->inode_id = file_table[i].inode_id;
            // read inode from inode table
            int inode_table_block_addr = inode_table_start_blk + ret->inode_id / 32;
            int inode_table_index = ret->inode_id % 32;
            unsigned char buf[BLOCK_SIZE];
            disk->read(inode_table_block_addr, buf);
            Inode *p = (Inode *)buf;
            ret->inode = p[inode_table_index];
            break;
        }
    }
    return ret;

}

bool FileSystem::CreateFile(int _file_id) {
    // allocate a inode
    unsigned int inode_id = alloc_inode();
    //Console::puts("allocate inode: ");Console::puti(inode_id);Console::puts("\n");
    if (inode_id == 255)
        return false;
    // write to file table
    file_table[file_table_curr].filename = _file_id;
    file_table[file_table_curr].inode_id = inode_id;
    file_table_curr++;
    // flush file table
    unsigned char buf[BLOCK_SIZE];
    FileList *fl_p = (FileList *)buf;
    for (int i = 0; i < 64; i++)
    {
        fl_p[i] = file_table[i];
    }
    disk->write(1, buf);
    return true;
}

bool FileSystem::DeleteFile(int _file_id) {
    File *fd;
    if ((fd = LookupFile(_file_id)) == NULL)
        return false;
    // 1, release all the data block in the file
    fd->Rewrite();

    // 2. release the inode
    assert(release_inode(fd->inode_id));
    delete fd;

    // 3. update the file table
    for (int i = 0; i <= file_table_curr; i++)
    {
        if(file_table[i].filename == _file_id)
        {
            //swap the last entry up
            file_table[i] = file_table[file_table_curr];
            file_table_curr--;
        }
    }
    // flush file table
    unsigned char buf[BLOCK_SIZE];
    FileList *fl_p = (FileList *)buf;
    for (int i = 0; i < 64; i++)
    {
        fl_p[i] = file_table[i];
    }
    disk->write(1, buf);

    return true;
}
