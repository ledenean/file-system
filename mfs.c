/**************************************************************
 * Class:  CSC-415-01 Fall 2022
 * Names: Pearl Anyanwu, Leo Saeteurn, Iza Limcolioc, Denean Le
 * Student IDs: 916832727, 920928746, 922004678, 921330745
 * GitHub Name:  ledenean
 * Group Name: PLID
 * Project: Basic File System
 *
 * File: mfs.c
 *
 * Description: Function implementation file
 *
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
#include "bitmap.h"
#include <dirent.h>

#define MAX_PATH 255
static char path[MAX_PATH];  // global variable for pathname (used for set and get cwd)

int findEntry(entry *de, char *pathname){
    for (int i = 0; i < NUM_OF_ENTRIES + 1; i++){
        if(strcmp(de[i].name, pathname)){
            return i;
        }
    }
    return -1;
}
int deUsed(entry *ptr){
    // assuming used and unused means its free or not free on bitmap
    // name would be null character if free
    if(strcmp(ptr, "\0") == 0){  
        return 1;
    }
    else{
        return 0;
    }
}

entry *loadDir(char *pathname){
    char *delim = "/";
    //printf("Entering loadDir function\n");
    entry *buffer = malloc(sizeof(entry)*NUM_OF_ENTRIES);    // 56 bytes
    //read directory from disk
    LBAread(buffer, 1, 6); 

    if (strcmp(pathname, "\0") != 0){
        if(strstr(path, pathname) == NULL){
            strcat(path, delim);
            strcat(path, pathname);
        }
    }
    else
    {
        for (int i = 0; i < strlen(buffer) + 1; i++)
        {
            strcat(path, delim);
            strcat(path, buffer[i].name);
        }
    }
    return buffer;
    
}

int parsePath(const char *pathname){
    char *delim = "/";
    char *token;
    char startPoint[MAX_PATH];
    struct entry *pp;  // pointer to parent
    char temp[2];
    int index = 0;

    strncpy(temp, pathname, 1);
    if(strcmp(temp, delim) != 0){
        // if relative then must load current working directory
        strcpy(startPoint, path);      
        
    }
    else{
        strcpy(startPoint, pathname);  //should be starting from root
    }
    
    token = strtok(startPoint, delim);
    while(token){
        pp = loadDir(token);

        if (strcmp(token, pp[index].name) != 0)
        {
            if (pp[index].fileType != FT_DIRECTORY)
            {
                printf("Not a directory type!\n");
            }
            printf("Directory not found!\n");
            return NULL;
        }

        index++;
        token = strtok(NULL, delim);
    }

    return index;
}


// Key directory functions

int fs_mkdir(const char *pathname, mode_t mode) 
{
    struct entry *de = malloc(sizeof(entry));
    // checks if directory exists
    int index = parsePath(pathname);

    // allocates blocks for directory and obtains starting location
    int startBlock = allocFreeSpace(de->blocksNeeded);
    // creates directory if directory doesn't exist
    if (index == NULL) {
        strcpy(de->name,pathname);
        de->location = startBlock;
        de->size += sizeof(de);
        de->fileType = FT_DIRECTORY;
        de->dateCreated = time(de->dateCreated);
    }
    printf("de = %s\n", de->name);
    LBAwrite(de, de->blocksNeeded, de->location);
    return 0;
}

int fs_rmdir(const char *pathname){
    // checks if directory exists
    struct entry *de = parsePath(pathname);

    // need to find number of entries to check if directory is empty
    int numOfEntries = de->size / sizeof(de);

    // checks if pathname is directory
    if(de != NULL && fs_isDir(de->fileType)){       
        if (numOfEntries != 0){
            printf("ERROR! Directory is not empty!\n");
            return -1;
        }
        else{
            // free blocks allocated for directory
            if(freeBlocksInFreeSpace(de->location, de->blocksNeeded) == 0){
                // mark as unused
                strcpy(de->name, "0");
                de->size = 0;
                de->location = 0;
                de->fileType = 0;
                // write this directory back out to disk
                LBAwrite(de, de->blocksNeeded , de->location);
                return 0;
            }
        }   
    }
}


// Directory iteration functions

fdDir * fs_opendir(const char *pathname){
    fdDir *fd;
    // initialize the entry dirPtr to be the pointer to the last element from parse path
    // since the dirPtr is connected to the directory entry
    struct entry *de;
    int index = parsePath(pathname);    // should return directory entry of the last element
    // // checks if it's a directory
    // if (fs_isDir(de->fileType) == 0){ 
    //     printf("Not a directory");
    //     exit;
    // }
    fd = malloc(sizeof(fdDir));   
    fd->dirInfo = malloc(sizeof(fdDir));
    fd->dirPtr = loadDir(de[index].name);           // loads from root dir
    fd->d_reclen = sizeof(fdDir);
    fd->dirEntryPosition = 0;


    // size of the entry = total bytes we have for the directory divide by dir entry size
    fd->numOfEntries = (de->size / sizeof(entry)); 
    // error handling
    if(fd < 0) {
        printf("Error opening file!");
        exit;
    }
    return fd;

}



struct fs_diriteminfo *fs_readdir(fdDir *dirp){
    for (int i = dirp->dirEntryPosition; i < dirp->numOfEntries; i++){
        // directory entry is used 
         if(deUsed(dirp->dirPtr[i].name) == 0){
            strcpy(dirp->dirInfo->d_name, dirp->dirPtr[i].name);
            dirp->dirInfo->fileType = FT_DIRECTORY;        
            dirp->dirInfo->d_reclen = sizeof(struct fs_diriteminfo);
            dirp->dirEntryPosition = i + 1;  // keeps track of where we left off
            return (dirp->dirInfo);
        }
    }
    return (NULL);
}; 



int fs_closedir(fdDir *dirp){
    // only thing we malloc'd other than fd 
    free(dirp->dirInfo);
    free(dirp);
    dirp = NULL;
    return 0;
}

// Misc directory functions

 char * fs_getcwd(char *pathname, size_t size){
    if(strlen(path) > size){
        return NULL;
    }
    if(strcmp(path, "\0") == 0){
        struct entry *de = loadDir(pathname);
        strcpy(pathname, path);
    }
    return pathname;
    
 }


 int fs_setcwd(char *pathname){   //linux chdir
    int de = parsePath(pathname);  // checks if pathname exists
    if(de != NULL){
        strcpy(path, pathname);  // sets global variable
        return 0;
    }
    return -1;  // returns error if path doesnt exist
 }


int fs_isFile(char * filename)	//return 1 if file, 0 otherwise
{   
    struct entry *de = parsePath(filename); 
    if(de->fileType == FT_REGFILE){
        return 1;
    }
    else{
        return 0;
    }

   
}


int fs_isDir(char * pathname)		//return 1 if directory, 0 otherwise
{
    struct entry *de = parsePath(pathname); 
    if(de->fileType == FT_DIRECTORY){
        return 1;
    }
    else{
        return 0;
    }
}

int fs_delete(char *filename)
{
    // this deletes a file (not dir) so we'd have to check if it's a file
    struct entry *de = parsePath(filename);
    if (de != NULL && fs_isFile(filename))
    { 
        // free blocks allocated for file
        if (freeBlocksInFreeSpace(de->location, de->blocksNeeded) == 0)
        {
            // mark as unused
            strcpy(de->name, "0");
            de->size = 0;
            de->location = 0;
            de->fileType = 0;
            // write this directory back out to disk
            LBAwrite(de, de->blocksNeeded, de->location);
            return 0;
        }
    }
    printf("ERROR! Cannot delete file : file not found or does not exist\n");
    return (-1);
}

int fs_stat(const char *path, struct fs_stat *buf) {
    struct entry *de;
    int index = findEntry(de, path);
    buf->st_size = de[index].size;
    buf->st_blocks = de[index].blocksNeeded;
    buf->st_createtime = de[index].dateCreated;
    buf->st_modtime = 0;
    buf->st_accesstime = 0;
}