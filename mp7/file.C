/*
     File        : file.C

     Author      : Riccardo Bettati
     Modified    : 2021/11/28

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
/* CONSTRUCTOR/DESTRUCTOR */
/*--------------------------------------------------------------------------*/

File::File(FileSystem *_fs, int _id) {
    Console::puts("Opening file.\n");
    id = _id;
    fs = _fs;
    // block_cache = new unsigned char[SimpleDisk::BLOCK_SIZE];
    inode = fs->LookupFile(id);
    if (inode == NULL){
        Console::puts("File does not exist!\n");
        assert(false);
    }
    // Console::puts("Inode address from file: ");
    // Console::putui((unsigned int)inode);
    // Console::puts("\nBuffer Address from file: ");
    // Console::putui((unsigned int)block_cache);
    // Console::puts("\n");
    inode->read_from_disk(&block_cache[0]);
    cur_pos = 0;
    // while(!EoF()) cur_pos += 1;
}

File::~File() {
    Console::puts("Closing file.\n");
    /* Make sure that you write any cached data to disk. */
    // Console::puti(inode->block_number);
    inode->write_to_disk(block_cache);
    // delete block_cache;
    /* Also make sure that the inode in the inode list is updated. */
}

/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

int File::Read(unsigned int _n, char *_buf) {
    Console::puts("reading from file\n");
    int characters_read = 0;
    for(int i=0; i < _n && !EoF(); i++){
        _buf[i] = block_cache[cur_pos];
        cur_pos ++;
        characters_read ++;
    }
    return characters_read;
}

int File::Write(unsigned int _n, const char *_buf) {
    Console::puts("writing to file\n");
    int characters_written = 0;
    for(int i=0;i < _n and cur_pos < SimpleDisk::BLOCK_SIZE; i++){
        block_cache[cur_pos] = _buf[i];
        characters_written ++;
        cur_pos ++;
    }
    if(cur_pos != SimpleDisk::BLOCK_SIZE){
        block_cache[cur_pos] = -1;
    }
    return characters_written;
    
}

void File::Reset() {
    Console::puts("resetting file\n");
    cur_pos = 0;
}

bool File::EoF() {
    if (cur_pos == SimpleDisk::BLOCK_SIZE || block_cache[cur_pos] == -1)
        return true;
    return false;
}
