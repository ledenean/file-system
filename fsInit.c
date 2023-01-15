/**************************************************************
 * Class:  CSC-415-01 Fall 2022
 * Names: Pearl Anyanwu, Leo Saeteurn, Iza Limcolioc, Denean Le
 * Student IDs: 916832727, 920928746, 922004678, 921330745
 * GitHub Name:  ledenean
 * Group Name: PLID
 * Project: Basic File System
 *
 * File: fsInit.c
 *
 * Description: Main driver for file system assignment.
 *
 * This file is where you will start and initialize your system
 *
 **************************************************************/



#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "fsInit.h"
#include "fsLow.h"
#include "bitmap.h"


///////// Initiate File System
int initFileSystem(uint64_t numberOfBlocks, uint64_t blockSize)
{
	printf("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	/* TODO: Add any code you need to initialize your file system. */
	
	struct VCB *buffer = malloc(blockSize); 	// VCB pointer
	LBAread(buffer, 1, 0);						// reading the VCB onto the disk in block 0

	// checking to see if the signature matches the defined magic number, if not, initialize VCB values,
	//	free space, and root directory
	if (buffer->signature != MAGIC_NUMBER)
	{
		buffer->numBlocks = numberOfBlocks; 	// 19531 blocks = 19531 bits = 2442 bytes
		buffer->sizeBlock = blockSize;			// 512 bytes
		
		// setting the signature to magic number so it matches after initializing and it doesn't 
		//	initialize the freespace and root directory again - can keep in memory
		buffer->signature = MAGIC_NUMBER;		

		// initializing the freeSpace to the return value from freeSpaceSystem
		buffer->freeSpace = initFreeSpaceSystem(numberOfBlocks, blockSize);

		// initializing the rootDir to the return value from initRoot
		buffer->rootDir = initRoot(numberOfBlocks, blockSize);
		// writing the VCB onto the disk in block 0		
		LBAwrite(buffer, 1, 0);
	}

	return 0;
}

///////// Initiate Free Space System
int initFreeSpaceSystem(uint64_t numberOfBlocks, uint64_t blockSize)
{
	// calculation to find the bytes needed based on the given value
	// referenced unit 06 lecture videos and slides, and class notes.
	// (19531 + (8 - 1)) / (8) = 2442.25 ; truncate .25 => 2442 bytes needed
	int bytesNeeded = ((numberOfBlocks + (BIT_SIZE - 1)) / (BIT_SIZE));	

	// (2442 + (512 - 1)) / (512) = 5.77 ; truncate .77 => 5 blocks needed
	int blocksNeeded = ((bytesNeeded + (blockSize - 1)) / (blockSize));

	bitmapSize = blocksNeeded * blockSize;
	// Allocates 2560 bytes for bitmap
	bitmap = malloc(bitmapSize); 

	// setting each block(bits) to be free
	for (int i = 0; i < bitmapSize; i++)
	{
		// Sets bit to 0 (free)
		clearBit(bitmap, i); 
	}

	// first 6 bits of bitmap allocated - VCB in block 0
	for (int i = 0; i < VCB_SIZE; i++)
	{
		// Sets bit to 1 (used)
		setBit(bitmap, i);
	}

	// storing bitmap into block 1
	LBAwrite(bitmap, blocksNeeded, 1);
	// Checks for free space in bitmap, ruint64_t numberOfBlocks, returns block number when found
	for (int m = 0; m < bitmapSize; m++)
	{
		int bit = checkBit(bitmap, m);
		if (bit == 0)
		{
			return m;
		}
	}
	return 0;
}

///////// Initiate Root Directory
int initRoot(uint64_t numberOfBlocks, uint64_t blockSize)
{
	int bytesNeeded = (sizeof(entry) * NUM_OF_ENTRIES);	
	entry *dirEntry = malloc(bytesNeeded); 

	// Calculates blocks needed for root
	int blocksNeeded = ((bytesNeeded + (blockSize - 1)) / (blockSize)); 
	for (int i = 0; i < (NUM_OF_ENTRIES + 1); i++)
	{	
		// Initializes each directory entry to be known free state
		strcpy(dirEntry[i].name, "\0");
		dirEntry[i].size = 0;
		dirEntry[i].blocksNeeded = blocksNeeded;
		dirEntry[i].location = 0;
		dirEntry[i].fileType = 0;
	}

	// Gets the starting block number inside the free space for root directory
	int startBlock = allocFreeSpace(blocksNeeded);
	if(startBlock == -1){
		printf("Error in allocating free space\n");
		exit(0);
	}
	// Sets first and second entries names, sizes, locations, and time stamps
	strcpy(dirEntry[0].name, ".");
	dirEntry[0].size = sizeof(entry);				
	dirEntry[0].location = startBlock;
	dirEntry[0].blocksNeeded = blocksNeeded;
	dirEntry[0].fileType = FT_DIRECTORY;
	dirEntry[0].dateCreated = time(dirEntry[0].dateCreated);
	
	// Second directory entry is same as first, except the name
	strcpy(dirEntry[1].name, "..");
	dirEntry[1].size = dirEntry[0].size;
	dirEntry[1].blocksNeeded = dirEntry[0].blocksNeeded;
	dirEntry[1].location = dirEntry[0].location;
	dirEntry[1].fileType = dirEntry[0].fileType;
	dirEntry[1].dateCreated = dirEntry[0].dateCreated;

	// Writes root directory onto disk
	LBAwrite(dirEntry, blocksNeeded, startBlock);
	// Returns block number of where root directory starts
	return startBlock;
}


///////// Exit File System
void exitFileSystem()
{
	printf("System exiting\n");
}