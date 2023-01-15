/**************************************************************
* Class:  CSC-415-01 Fall 2022
 * Names: Pearl Anyanwu, Leo Saeteurn, Iza Limcolioc, Denean Le
 * Student IDs: 916832727, 920928746, 922004678, 921330745
 * GitHub Name: ledenean
 * Group Name: PLID
*
* File: fsInit.h
*
* Description: 
*	This is the file system initialization interface.
*	This is the interface needed by the driver to interact with
*	your filesystem.
*
**************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "fsLow.h"
#include "mfs.h"
#include <dirent.h>

///////// Define Values
#define MAGIC_NUMBER 0x504C4944   // name of our group, PLID, in hex
#define VCB_SIZE 6		  // size of VCB when being initialized

///////// Volume Control Block Structure
typedef struct VCB
{
	int numBlocks;	 	// Total number of blocks
	int sizeBlock;	 	// Size of blocks
	int freeSpace;	 	// Free block position 
	int rootDir;	 	// Pointer to root directory
	long signature;	 	// Unique magic number
} VCB;

///////// Function Prototypes
int initFreeSpaceSystem(uint64_t numberOfBlocks, uint64_t blockSize);
int initRoot(uint64_t numberOfBlocks, uint64_t blockSize);
