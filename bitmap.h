/**************************************************************
* Class:  CSC-415-01 Fall 2022
* Names: Pearl Anyanwu, Leo Saeteurn, Iza Limcolioc, Denean Le
* Student IDs: 916832727, 920928746, 922004678, 921330745
* GitHub Name: ledenean
 * Group Name: PLID
 * Project: Basic File System
*
* File: bitmap.h
*
* Description: This is the header file for the bitmap.
*
**************************************************************/
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#ifndef uint64_t
typedef u_int64_t uint64_t;
#endif
#ifndef uint32_t
typedef u_int32_t uint32_t;
#endif
#define BIT_SIZE 8		  // each block has 8 bits

///////// Pointer for the bitmap
uint64_t *bitmap;
int bitmapSize;

void setBit(uint64_t bitmap[], int blockNum);
void clearBit(uint64_t bitmap[], int blockNum);
int checkBit(uint64_t bitmap[], int blockNum);
int allocFreeSpace(uint64_t numberOfBlocks);
int freeBlocksInFreeSpace(uint64_t location, uint64_t size);
