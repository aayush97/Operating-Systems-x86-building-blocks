/*
     File        : file_system.C

     Author      : Riccardo Bettati
     Modified    : 2021/11/28

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
/* CLASS Inode */
/*--------------------------------------------------------------------------*/

/* You may need to add a few functions, for example to help read and store 
   inodes from and to disk. */

/*--------------------------------------------------------------------------*/
/* CLASS FileSystem */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

FileSystem::FileSystem() {
    Console::puts("In file system constructor.\n");
    inodes = new Inode[MAX_INODES];
}

FileSystem::~FileSystem() {
    Console::puts("unmounting file system\n");
    /* Make sure that the inode list and the free list are saved. */
    disk->write(0, (unsigned char *)inodes);
    disk->write(1, free_blocks);
    delete inodes;
}


/*--------------------------------------------------------------------------*/
/* FILE SYSTEM FUNCTIONS */
/*--------------------------------------------------------------------------*/


bool FileSystem::Mount(SimpleDisk * _disk) {
    Console::puts("mounting file system from disk\n");

    /* Here you read the inode list and the free list into memory */
    
    disk = _disk;
    // unsigned char inode_block[FileSystem::BLOCK_SIZE];
    disk->read(0, (unsigned char *) inodes);
    // inodes = (Inode *) inode_block;
    for(int i=0; i<MAX_INODES; i++){
        inodes[i].fs = this;
    }
    free_blocks = new unsigned char[FileSystem::BLOCK_SIZE];
    disk->read(1, free_blocks);

    size = disk->size() / FileSystem::BLOCK_SIZE;
    return true;

}

bool FileSystem::Format(SimpleDisk * _disk, unsigned int _size) { // static!
    Console::puts("formatting disk\n");
    /* Here you populate the disk with an initialized (probably empty) inode list
       and a free list. Make sure that blocks used for the inodes and for the free list
       are marked as used, otherwise they may get overwritten. */
    unsigned char * free_blocks = new unsigned char[FileSystem::BLOCK_SIZE];
    Inode* inodes = new Inode[MAX_INODES];
    for(int i=0; i<MAX_INODES;i++){
        inodes[i].free = true;
        inodes[i].block_number = -1;
    }
    free_blocks[0] = 0;
    free_blocks[1] = 0;
    for (int i=2; i<FileSystem::BLOCK_SIZE;i++){
        free_blocks[i] = 1;
    }
    _disk->write(0, (unsigned char *) inodes);
    _disk->write(1, free_blocks);
    delete free_blocks;
    delete inodes;
    return true;
}

Inode * FileSystem::LookupFile(int _file_id) {
    Console::puts("looking up file with id = "); Console::puti(_file_id); Console::puts("\n");
    /* Here you go through the inode list to find the file. */
    for(int i=0; i<MAX_INODES; i++){
        // Console::puts("Id: ");
        // Console::puti(inodes[i].id);
        // Console::puts("\nBlock Number: ");
        // Console::puti(inodes[i].block_number);
        // Console::puts("\nFree value: ");
        // Console::puti(inodes[i].free);
        // Console::puts("\n");
        if (inodes[i].id == _file_id && !inodes[i].free){
            return &inodes[i];
        }
    }
    return NULL;
}

bool FileSystem::CreateFile(int _file_id) {
    Console::puts("creating file with id:"); Console::puti(_file_id); Console::puts("\n");
    /* Here you check if the file exists already. If so, throw an error.
       Then get yourself a free inode and initialize all the data needed for the
       new file. After this function there will be a new file on disk. */
    if (LookupFile(_file_id) != NULL){
            Console::puts("File already exists");
            return false;
    }

    int free_inode_idx = 0;
    for(; free_inode_idx < MAX_INODES; free_inode_idx++){
        if(inodes[free_inode_idx].free) break;
    } 
    inodes[free_inode_idx].id = _file_id;
    inodes[free_inode_idx].free = 0;
    inodes[free_inode_idx].fs = this;

    // find a free block
    for(int i=0; i<FileSystem::BLOCK_SIZE; i++){
        if(free_blocks[i] == 1){
            free_blocks[i] = 0;
            inodes[free_inode_idx].block_number = i;
            break; 
        }
    }
    return true;
}

bool FileSystem::DeleteFile(int _file_id) {
    Console::puts("deleting file with id:"); Console::puti(_file_id); Console::puts("\n");
    /* First, check if the file exists. If not, throw an error. 
       Then free all blocks that belong to the file and delete/invalidate 
       (depending on your implementation of the inode list) the inode. */
    for (int i = 0; i < MAX_INODES; i++)
    {
        if (inodes[i].id == _file_id && !inodes[i].free)
        {
            inodes[i].free = true;
            return true;
        }
    }
    Console::puts("File not found for deletion\n");
    return false;
}


void Inode::read_from_disk(unsigned char *_buf){
    // Console::puts("Reading inode information from disk block number = ");
    // Console::puti(block_number);
    // Console::puti(free);
    // Console::puts("\n");
    // Console::puts("Inode address: ");
    // Console::putui((unsigned int) this);
    // Console::puts("\nBuffer Address: ");
    // Console::putui((unsigned int) _buf);
    // Console::puts("\n");
    fs->disk->read(block_number, _buf);
    // Console::puts("Reading inode information from disk block number = ");
    // Console::puti(block_number);
    // Console::puti(free);
    // Console::puts("\n");
}

void Inode::write_to_disk(unsigned char *_buf){
    // Console::puts("Writing inode infromation to disk block number = ");
    // Console::puti(block_number);
    // Console::puts("\n");
    fs->disk->write(block_number, _buf);
}