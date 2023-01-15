/**************************************************************
* Class:  CSC-415-01 Fall 2022
* Names: Pearl Anyanwu, Leo Saeteurn, Iza Limcolioc, Denean Le 
* Student IDs: 916832727, 920928746, 922004678, 921330745
* GitHub Name:  ledenean
* Group Name: PLID
* Project: Basic File System
*
* File: b_io.c
*
* Description: Key File I/O
*
**************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>			// for malloc
#include <string.h>			// for memcpy
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "b_io.h"
#include "mfs.h"
#include "fsLow.h"
#include "bitmap.h"

#define MAXFCBS 20
#define B_CHUNK_SIZE 512

typedef struct b_fcb
	{
	/** TODO add all the information you need in the file control block **/
	fdDir * fi; 		// holds a pointer to the associated file info
	char * buf;			// holds the open file buffer
	int index;			// holds the current position in the buffer
	int buflen;			// holds how many valid bytes are in the buffer
	int currentBlock; 	// holds current block number
	int numBlocks; 		// holds how many blocks the file occupies
	} b_fcb;
	
b_fcb fcbArray[MAXFCBS];

int startup = 0;		// Indicates that this has not been initialized

//Method to initialize our file system
void b_init ()
	{
	//init fcbArray to all free
	for (int i = 0; i < MAXFCBS; i++)
		{
		fcbArray[i].buf = NULL; //indicates a free fcbArray
		}
		
	startup = 1;
	}

//Method to get a free FCB element
b_io_fd b_getFCB ()
	{
	for (int i = 0; i < MAXFCBS; i++)
		{
		if (fcbArray[i].buf == NULL)
			{
			return i;		//Not thread safe (But do not worry about it for this assignment)
			}
		}
	return (-1);  //all in use
	}
	
// Interface to open a buffered file
// Modification of interface for this assignment, flags match the Linux flags for open
// O_RDONLY, O_WRONLY, or O_RDWR
// RDONLY, WRONLY, RDWR, CREATE, TRUNC
b_io_fd b_open (char * filename, int flags)
	{
	int fd;					
	int returnFd;
	b_fcb *fi;
	char * buf;
	//*** TODO ***:  Modify to save or set any information needed
		
	if (startup == 0) b_init();  			//Initialize our system

	fi = getFileInfo(parsePath(filename)); 	// getting file info using parsePath
	if (fi == NULL) {
		return -2; 							// checking if file not found
	}

	// malloc buffer, if cant malloc buffer open fails
	buf = malloc(B_CHUNK_SIZE);
	if(buf == NULL) {
		return -1;
	}

	returnFd = b_getFCB();				// get our own file descriptor
										// check for error - all used FCB's	
	fcbArray[fd].fi = fi; 
	fcbArray[fd].buf = buf;
	fcbArray[fd].index = 0;
	fcbArray[fd].buflen = 0;
	fcbArray[fd].currentBlock = 0;
	fcbArray[fd].numBlocks = (fcbArray[fd].numBlocks + (B_CHUNK_SIZE - 1)) / B_CHUNK_SIZE; 

	// to test if works with printf statement
	printf("File '%s' opened with fd %d\n", filename, fd);

	return (returnFd);						// all set
	}
	// ---- extra logic
	// write own version of GetFileInfo
	// main function that function's gonna use is parsePath
	// parsePath gets the directory entry so it is the "key"
	// use that ptr and populate our own GetFileInfo


// Interface to seek function	
// using nth byte by changing location of pointer of a fd
// allows file offset to set beyond EOF (based on manpage)
int b_seek (b_io_fd fd, off_t offset, int whence)
	{
	if (startup == 0) b_init();  		//Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
		
	off_t newPos; 						// temp variable -> "nth" byte

	switch(whence) {
		case SEEK_SET: 					// file offset set to offset bytes
			newPos = offset;
			break;
		case SEEK_CUR: 					// file offset set to current location plus offset bytes
			newPos = fcbArray[fd].currentBlock + offset;
			// newPos = fd->offset + offset;
			break;
		case SEEK_END: 					// file offset set to block size of file plus offset bytes
			newPos = fcbArray[fd].numBlocks + offset;
			// newPos = fd->fileSize + offset;
			break;
		default:
			return -1;
	}
		
	}
	// ---- extra logic
	// this function repositions the file pointer. 32:33
	// 1. read block.
	// 2. stick block into buffer.
	// 3. set index.
	// 4. readjust fcb.


// Interface to write function	
int b_write (b_io_fd fd, char * buffer, int count)
	{
	if (startup == 0) b_init();  		//Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}


	if (count <= 0) {
		printf("Zero length");
		return (-1);
	}
	
	buffer = malloc(count);

	// allocate blocks -> dont know how many to allocate
	// solution : start with random num(?), if too much, release remaining into free space
	// if need more, allocate more
	// make sure to check if we have enough in freespace to allocate
	
	int blocksWritten;
	int freeBlocks = 0;
	int numOfBlocks = 50;
	int startBlock = allocFreeSpace(numOfBlocks);
	if(startBlock == 0){
		printf("Not enough blocks available");
		// finding how many free blocks are left
		for (int i = 0; i < bitmapSize; i++)
		{
			int bit = checkBit(bitmap, i);
			if(bit == 0){
				freeBlocks++;
			}
		}
		// adjusting number of blocks to leftover free blocks in bitmap
		numOfBlocks = freeBlocks;
	}

	 
	while(count == numOfBlocks){
		numOfBlocks *= 2; // double amount of space
		blocksWritten += LBAwrite(buffer, count, startBlock);
	}
	
	if (count < numOfBlocks)
	{
		int blocksLeft = numOfBlocks - blocksWritten;
		// release remaining blocks into free space
		for (int i = startBlock + blocksWritten; i < blocksLeft; i++)
		{
			clearBit(bitmap, i);
		}
	}	
	if(count > numOfBlocks){
		blocksWritten += LBAwrite(buffer, count, startBlock);  
	}
	

	return blocksWritten; 
	}



// Interface to read a buffer

// Filling the callers request is broken into three parts
// Part 1 is what can be filled from the current buffer, which may or may not be enough
// Part 2 is after using what was left in our buffer there is still 1 or more block
//        size chunks needed to fill the callers request.  This represents the number of
//        bytes in multiples of the blocksize.
// Part 3 is a value less than blocksize which is what remains to copy to the callers buffer
//        after fulfilling part 1 and part 2.  This would always be filled from a refill 
//        of our buffer.
//  +-------------+------------------------------------------------+--------+
//  |             |                                                |        |
//  | filled from |  filled direct in multiples of the block size  | filled |
//  | existing    |                                                | from   |
//  | buffer      |                                                |refilled|
//  |             |                                                | buffer |
//  |             |                                                |        |
//  | Part1       |  Part 2                                        | Part3  |
//  +-------------+------------------------------------------------+--------+
int b_read (b_io_fd fd, char * buffer, int count)
	{
	int bytesRead; 				// for reading the bytes
	int bytesReturn; 			// the bytes we will return
	int pt1, pt2, pt3; 			// holds 3 copy lengths
	int numBlocksCopied; 		// holds number of whole blocks needed
	int remainBytesInBuffer; 	// holds how many bytes left in buffer

	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 			//invalid file descriptor
		}
		
	// check that specified FCB is in use
	if(fcbArray[fd].fi = NULL) {
		return -1; 				// file not open
	}

	// number of bytes that is available to copy from buffer
	remainBytesInBuffer = fcbArray[fd].buflen - fcbArray[fd].index;

	// limiting count to the file length / end of file
	int amountDelivered = (fcbArray[fd].currentBlock * B_CHUNK_SIZE) - remainBytesInBuffer;

	if((count + amountDelivered) > fcbArray[fd].numBlocks) {
		count = fcbArray[fd].numBlocks - amountDelivered; 

		if(count < 0) { 		// check if count is negative
			// printf statement to debug
			printf("Error! Count is negative, cannot exceed or go past EOF");
		}
	}

	if(remainBytesInBuffer >= count) { 		// checking if we have enough in buffer to give (satisfying request)
		pt1 = count; 						// completely buffered (requested count)
		pt2 = 0;
		pt3 = 0; 							// "next" bytes not needed
	}
	else { 									// otherwise have to give more than what is in buffer
		pt1 = remainBytesInBuffer; 			// whatever is left in the buffer
		pt3 = count - remainBytesInBuffer; 	// how many more bytes we need to give

		numBlocksCopied = pt3 / B_CHUNK_SIZE; // how many whole blocks there are
		pt2 = numBlocksCopied * B_CHUNK_SIZE; // how many whole blocks worth of bytes to give

		pt3 = pt3 - pt2; 					  // giving "remainder", how many bytes after given whole blocks
	}

	if(pt1 > 0) { 							 // memcpy part 1
		memcpy(buffer, fcbArray[fd].buf + fcbArray[fd].index, pt1); 	// copy those to buffer
		fcbArray[fd].index = fcbArray[fd].index + pt1; 					// increment index by that much
	}

	if(pt2 > 0) { 							// blocks copied directly to callers buffer
		// limiting blocks to the blocks left
		bytesRead = LBAread(buffer+pt1, numBlocksCopied, fcbArray[fd].currentBlock + fcbArray[fd].index);
		fcbArray[fd].currentBlock += numBlocksCopied;
		pt2 = bytesRead; 					// might be less than this if we hit end of file
	}

	if(pt3 > 0) { 							// have to refill our buffer to copy more blocks
		// reading B_CHUNK_SIZE bytes into buffer
		bytesRead = LBAread(fcbArray[fd].buf, 1, fcbArray[fd].currentBlock + fcbArray[fd].index);
		fcbArray[fd].currentBlock += 1;
		// did a read into buffer -> reset the offset
		fcbArray[fd].index = 0;
		fcbArray[fd].buflen = bytesRead; 	// how many bytes are actually read

		if(bytesRead < pt3) { 				// checking if there's not enough left to satisfy
			pt3 = bytesRead;
		}

		if(pt3 > 0) { 						// memcpy bytesRead
			memcpy(buffer+pt1+pt2, fcbArray[fd].buf + fcbArray[fd].index, pt3);
			fcbArray[fd].index = fcbArray[fd].index + pt3; 	// adjusting index for copied bytes
		}
	}

	bytesReturn = pt1 + pt2 + pt3; 			// sum of all parts
	return (bytesReturn);
	}
	
// Interface to Close the file	
int b_close (b_io_fd fd)
	{
		// frees allocated memory (buffer)
		// places file control block to unused FCB pool
		b_fcb * fcb = &fcbArray[fd];
		printf("File closing %d\n", fd);
		free(fcbArray[fd].buf);
		fcbArray[fd].buf = NULL;
		fcbArray[fd].fi = NULL;
		return 0;

	}
