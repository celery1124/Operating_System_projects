/*
     File        : file.H

     Author      : Riccardo Bettati
     Modified    : 2017/05/01

     Description : Simple File class with sequential read/write operations.
 
*/

#ifndef _FILE_H_
#define _FILE_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

#define uint16_t unsigned short
#define uint32_t unsigned int
#define BLOCK_SIZE 512

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* class  i n o d e   */
/*--------------------------------------------------------------------------*/

class FileSystem;

class Inode {
    public:
        uint32_t size;
        uint16_t direct_index[5];
        uint16_t indirect_index;
    };

/*--------------------------------------------------------------------------*/
/* class  F i l e   */
/*--------------------------------------------------------------------------*/

class File  {
    friend class FileSystem;
    
private:
    /* -- your file data structures here ... */
    unsigned int inode_id;
    Inode inode;
    uint32_t curr_pointer;
    
    FileSystem *fs;
    /* -- maybe it would be good to have a reference to the file system? */
    
public:

    File(FileSystem *_fs);
    /* Constructor for the file handle. Set the ’current
     position’ to be at the beginning of the file. */
    
    int Read(unsigned int _n, char * _buf);
    /* Read _n characters from the file starting at the current location and
     copy them in _buf.  Return the number of characters read. 
     Do not read beyond the end of the file. */
    
    void Write(unsigned int _n, const char * _buf);
    /* Write _n characters to the file starting at the current location, 
     if we run past the end of file, 
     we increase the size of the file as needed. */
    
    void Reset();
    /* Set the ’current position’ at the beginning of the file. */
    
    void Rewrite();
    /* Erase the content of the file. Return any freed blocks.
     Note: This function does not delete the file! It just erases its content. */
    
    bool EoF();
    /* Is the current location for the file at the end of the file? */

};

#endif
