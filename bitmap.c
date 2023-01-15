

/**************************************************************
 * Class:  CSC-415-01 Fall 2022
 * Names: Pearl Anyanwu, Leo Saeteurn, Iza Limcolioc, Denean Le
 * Student IDs: 916832727, 920928746, 922004678, 921330745
 * GitHub Name:  ledenean
 * Group Name: PLID
 * Project: Basic File System
 *
 * File: bitmap.c
 *
 * Description: Bitmap implementation file
 *
 *
 **************************************************************/

#include "bitmap.h"


///////// Manipulate Bitmap Functions
// using bitwise operators to manipulate bits
// referenced this site to implement the functions setBit, 
//	https://www.geeksforgeeks.org/set-clear-and-toggle-a-given-bit-of-a-number-in-c/


// set bits to 1 for allocating bits as NOT free
void setBit(uint64_t bitmap[], int blockNum)
{
	bitmap[blockNum / BIT_SIZE] |= (1 << blockNum % BIT_SIZE);
}

// negating the bits that are 1s to 0s which are free bits
void clearBit(uint64_t bitmap[], int blockNum)
{
	bitmap[blockNum / BIT_SIZE] &= ~(1 << blockNum % BIT_SIZE);
}

// checking the each bit value in the bitmap 
// using this function to check for the value of 0 as the next free space
int checkBit(uint64_t bitmap[], int blockNum)
{
	int bit = (bitmap[blockNum / BIT_SIZE] >> (blockNum % BIT_SIZE)) & 1;
	return bit;
}

///////// Allocating Free Space
int allocFreeSpace(uint64_t numberOfBlocks){
	int startBlock = 0;
	int usedBlocks = 0;

	// Finds first free bit
	for (int i = 0; i < bitmapSize; i++)
	{
		if (checkBit(bitmap, i) == 0)
		{
			startBlock = i;
			break;
		}
	}
	// sets bits as used
	for (int j = startBlock; j < startBlock + numberOfBlocks; j++)
	{
		setBit(bitmap, j);
	}
	// checks if bits are set as used
	for(int k = startBlock; k < startBlock + numberOfBlocks; k++){
		if(checkBit(bitmap, k) == 1){
			usedBlocks++;
		}
	}
	if(usedBlocks != numberOfBlocks){
		// Returns -1 if any errors occur
		return -1;		
	}

	// Returns start block after confirming enough blocks are available
	return startBlock;  
}


int freeBlocksInFreeSpace(uint64_t location, uint64_t size){
	int freeBlocks = 0;
	// Free bits by setting bits to zero
	for(int i = location; i < location + size; i++){
		clearBit(bitmap, i);
	}

	// Checks if bits are freed properly
	for(int j = location; j < location + size; j++){
		int bit = checkBit(bitmap, j);
		if(bit == 1){
			freeBlocks++;
		}
	}
	// Returns 0 if successful, otherwise returns -1
	if(freeBlocks != size){
		return -1;
	}
	return 0;

}