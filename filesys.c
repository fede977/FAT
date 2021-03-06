/* filesys.c
 * 
 * provides interface to virtual disk
 * 
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "filesys.h"


diskblock_t  virtualDisk [MAXBLOCKS] ;           // define our in-memory virtual, with MAXBLOCKS blocks
fatentry_t   FAT         [MAXBLOCKS] ;           // define a file allocation table with MAXBLOCKS 16-bit entries
fatentry_t   rootDirIndex            = 0 ;       // rootDir will be set by format
direntry_t * currentDir              = NULL ;
fatentry_t   currentDirIndex         = 0 ;

/* writedisk : writes virtual disk out to physical disk
 * 
 * in: file name of stored virtual disk
 */

void writedisk ( const char * filename )
{
   printf ( "writedisk> virtualdisk[0] = %s\n", virtualDisk[0].data ) ;
   FILE * dest = fopen( filename, "w" ) ;
   if ( fwrite ( virtualDisk, sizeof(virtualDisk), 1, dest ) < 0 )
      fprintf ( stderr, "write virtual disk to disk failed\n" ) ;
   //write( dest, virtualDisk, sizeof(virtualDisk) ) ;
   fclose(dest) ;
   
}

void readdisk ( const char * filename )
{
   FILE * dest = fopen( filename, "r" ) ;
   if ( fread ( virtualDisk, sizeof(virtualDisk), 1, dest ) < 0 )
      fprintf ( stderr, "write virtual disk to disk failed\n" ) ;
   //write( dest, virtualDisk, sizeof(virtualDisk) ) ;
      fclose(dest) ;
}


/* the basic interface to the virtual disk
 * this moves memory around
 */

void writeblock ( diskblock_t * block, int block_address )
{
   //printf ( "writeblock> block %d = %s\n", block_address, block->data ) ;
   memmove ( virtualDisk[block_address].data, block->data, BLOCKSIZE ) ;
   //printf ( "writeblock> virtualdisk[%d] = %s / %d\n", block_address, virtualDisk[block_address].data, (int)virtualDisk[block_address].data ) ;
}


/* read and write FAT
 * 
 * please note: a FAT entry is a short, this is a 16-bit word, or 2 bytes
 *              our blocksize for the virtual disk is 1024, therefore
 *              we can store 512 FAT entries in one block
 * 
 *              how many disk blocks do we need to store the complete FAT:
 *              - our virtual disk has MAXBLOCKS blocks, which is currently 1024
 *                each block is 1024 bytes long
 *              - our FAT has MAXBLOCKS entries, which is currently 1024
 *                each FAT entry is a fatentry_t, which is currently 2 bytes
 *              - we need (MAXBLOCKS /(BLOCKSIZE / sizeof(fatentry_t))) blocks to store the
 *                FAT
 *              - each block can hold (BLOCKSIZE / sizeof(fatentry_t)) fat entries
 */
void copyFAT(){
    int fatblocksneeded = (MAXBLOCKS/FATENTRYCOUNT);

    for(int i = 0; i<fatblocksneeded; i++){
        diskblock_t block;
        for(int j =0; j<FATENTRYCOUNT; j++){
            block.fat[j] = FAT[j+i*FATENTRYCOUNT];
        }
        writeblock(&block, i+1);
    }
}

/* implement format()
 */
void format ( )
{
   diskblock_t block ;
   direntry_t  rootDir ;

   int         pos             = 0 ;
   int         fatentry        = 0 ;
   int         fatblocksneeded =  (MAXBLOCKS / FATENTRYCOUNT ) ;

   /* prepare block 0 : fill it with '\0',
    * use strcpy() to copy some text to it for test purposes
	* write block 0 to virtual disk
	*/
    for(int i = 0; i<BLOCKSIZE; i++){
        block.data[i] = '\0';
    }

	/* prepare FAT table
	 * write FAT blocks to virtual disk
	 */
    strcpy(block.data, "CS3026 OPERATING SYSTEM ASSESSMENT");
    writeblock(&block, 0);

	 /* prepare root directory
	  * write root directory block to virtual disk
	  */
    for(int j = 0; j<=FATENTRYCOUNT*fatblocksneeded; j++){
        FAT[j] = UNUSED;
    }

    FAT[0] = ENDOFCHAIN;
    FAT[1] = 2;
    FAT[2] = ENDOFCHAIN;
    FAT[3] = ENDOFCHAIN;

    copyFAT();
    

    for(int i = 0; i<BLOCKSIZE; i++){
        block.data[i] = '\0';
    }

    block.dir.isdir = 0;
    block.dir.nextEntry = 1;

    writeblock(&block, fatblocksneeded+1);

    rootDirIndex = fatblocksneeded+1;
}

int allocFAT(){
    for(int i = 0; i<MAXBLOCKS; i++){
        if(FAT[i] == UNUSED){
            FAT[i] = ENDOFCHAIN;
            copyFAT();
            return i;
            break;
        }
    }
}


MyFILE * myfopen(char * fileName, const char * mode){
    diskblock_t block;

    block = virtualDisk[3];
    MyFILE * File = malloc(sizeof(MyFILE));
    strcpy(File->mode,mode);
    int filePos;
    int i, fileFound = 0;
    for(i = 0; i<DIRENTRYCOUNT; i++){
        if(block.dir.entrylist[i].unused){
            break;
        }else if(strcmp(block.dir.entrylist[i].name, fileName) == 0){
            filePos = i;
            fileFound = 1;
            break;
        }
    }

    if(fileFound){
        File ->blockno = block.dir.entrylist[filePos].firstblock;
        File -> pos = 0;

    }else{
        for(i = 0; i<DIRENTRYCOUNT; i++){
            if(block.dir.entrylist[i].unused){
                break;
            }
        }
        filePos = allocFAT();

        File->blockno = filePos;
        block.dir.entrylist[i].firstblock = filePos;

        copyFAT();

        strcpy(block.dir.entrylist[i].name, fileName);
        block.dir.entrylist[i].unused = 0;
        writeblock(&block, 3);
    }
    return(File);
}

void myfputc(int b, MyFILE * stream){

    int freeSpace, newPos;
    int position = stream->blockno;

    while(!freeSpace){
        if(FAT[position] == ENDOFCHAIN){
            freeSpace = 1;
        }else{
            position = FAT[position];
        }
    }
    stream->buffer = virtualDisk[position];

    for(int i= 0; i<BLOCKSIZE; i++){
        if(stream->buffer.data[i] == '\0'){
            stream->pos = i;
            break;
        }
    }

    stream -> buffer.data[stream->pos] = b;
    writeblock(&stream->buffer, position);
    stream -> pos += 1;

    if(stream->pos == BLOCKSIZE){
        stream->pos = 0;
        newPos = allocFAT();
        FAT[position] = newPos;
        copyFAT();
        writeblock(&stream->buffer, stream -> blockno);
        stream->blockno = newPos;
        for(int i = 0; i<MAXBLOCKS; i++){
            stream->buffer.data[i] = '\0';
        }
    }

}

void myfclose(MyFILE * stream){
    writeblock(&stream->buffer, stream -> blockno);
    free(stream);
}

/* use this for testing
 */

void printBlock ( int blockIndex )
{
   printf ( "virtualdisk[%d] = %s\n", blockIndex, virtualDisk[blockIndex].data ) ;
}

