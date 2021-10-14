/* This file contains functions that are not part of the visible "interface".
 * They are essentially helper functions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simfs.h"

/* Internal helper functions first.
 */

int blocks_index = 0;
int used_blocks[MAXBLOCKS];

FILE *
openfs(char *filename, char *mode)
{
    FILE *fp;
    if((fp = fopen(filename, mode)) == NULL) {
        perror("openfs");
        exit(1);
    }
    return fp;
}

void
closefs(FILE *fp) 
{
    if(fclose(fp) != 0) {
        perror("closefs");
        exit(1);
    }
}

void
findusedblocks(int *used_blocks, int reference, fnode *fnodes){

    if (fnodes[reference].nextblock == -1) {

        return;
    }
    used_blocks[blocks_index] = fnodes[reference].nextblock;
    blocks_index += 1;
    findusedblocks(used_blocks, fnodes[reference].nextblock, fnodes);

}



/* File system operations: creating, deleting, reading, and writing to files.
 */

//CREATING A FILE
void createfile(char *filename, char *simfilename) {

    fentry files[MAXFILES];
    fnode fnodes[MAXBLOCKS];

    int i;

    // output error is file name exceeds 11 characters
    if (strlen(simfilename) > 11) {
        fprintf(stderr, "Error: not enough space for new file\n");
        exit(1);
    }
    
    //read fentries and fnodes into files and fnodes arrays, respectively.

    FILE *fp ;
    fp = openfs(filename, "r");

    if ((fread(files, sizeof(fentry), MAXFILES, fp)) == 0) {
        fprintf(stderr, "Error: could not read file entries\n");
        closefs(fp);
        exit(1);
    }

    if ((fread(fnodes, sizeof(fnode), MAXBLOCKS, fp)) == 0) {
        fprintf(stderr, "Error: could not read fnodes\n");
        closefs(fp);
        exit(1);
    }

    //Number of blocks used
    int bytes_used = sizeof(fentry) * MAXFILES + sizeof(fnode) * MAXBLOCKS;
    int bytes_to_write = (BLOCKSIZE - (bytes_used % BLOCKSIZE)) % BLOCKSIZE;
    int blocks_used = (bytes_used + bytes_to_write)/ BLOCKSIZE;
    char blocks[MAXBLOCKS - blocks_used][BLOCKSIZE];

    fseek(fp, bytes_to_write, SEEK_CUR);
    
    // reading file blocks from the file
    //FILE DATA
    for (i = 0 ; i < MAXBLOCKS - blocks_used ; i++){
        if ((fread(blocks[i], 1, BLOCKSIZE, fp)) == 0 ){
            fprintf(stderr, "Error: write failed on init\n");
            closefs(fp);
            exit(1);
        } 
    }

    closefs(fp);


    //output error if the file name already exists


    for(i = 0; i < MAXFILES; i++){
        if (strcmp(files[i].name, simfilename) == 0){
            fprintf(stderr, "Error: Duplicate name\n");
            exit(1);
        }
    }
    // Find first available fentry index
    int fentry_index = 0;
    for (fentry_index = 0; fentry_index < MAXFILES; fentry_index++) {
        if (strlen(files[fentry_index].name) == 0) {
            break;
        }
    }

    //output error if fentries are full

    if(fentry_index == MAXFILES) {
        fprintf(stderr, "Error: not enough space for new file\n");
        exit(1);
    }
   
   // change file name in files
    strcpy(files[fentry_index].name, simfilename);

    // write updated fentries and fnodes in the smfs

    fp = openfs(filename, "w");

    if(fwrite(files, sizeof(fentry), MAXFILES, fp) < MAXFILES) {
        fprintf(stderr, "Error: write failed on init\n");
        closefs(fp);
        exit(1);
    }

    if(fwrite(fnodes, sizeof(fnode), MAXBLOCKS, fp) < MAXBLOCKS) {
        fprintf(stderr, "Error: write failed on init\n");
        closefs(fp);
        exit(1);
    }


    fseek(fp, bytes_to_write, SEEK_CUR);

    //writing file blocks to the file
    //FILE DATA
    for (i = 0 ; i < MAXBLOCKS - blocks_used  ; i++){
        if (fwrite(blocks[i], BLOCKSIZE, 1, fp) < 1) {
            fprintf(stderr, "Error: write failed on init\n");
            closefs(fp);
            exit(1);
        } 
    }
    closefs(fp);
    
}


//DELETING A FILE
void deletefile(char *filename, char *simfilename) {


    fentry files[MAXFILES];
    fnode fnodes[MAXBLOCKS];
    int i;

    // output error is file name exceeds 11 characters
    if (strlen(simfilename) > 11) {
        fprintf(stderr, "Error: not enough space for new file\n");
        exit(1);
    }
    
    //read fentries and fnodes into files and fnodes arrays, respectively.

    FILE *fp ;
    fp = openfs(filename, "r");

    if ((fread(files, sizeof(fentry), MAXFILES, fp)) == 0) {
        fprintf(stderr, "Error: could not read file entries\n");
        closefs(fp);
        exit(1);
    }

    if ((fread(fnodes, sizeof(fnode), MAXBLOCKS, fp)) == 0) {
        fprintf(stderr, "Error: could not read fnodes\n");
        closefs(fp);
        exit(1);
    }

    //Number of blocks used
    int bytes_used = sizeof(fentry) * MAXFILES + sizeof(fnode) * MAXBLOCKS;
    int bytes_to_write = (BLOCKSIZE - (bytes_used % BLOCKSIZE)) % BLOCKSIZE;
    int blocks_used = (bytes_used + bytes_to_write)/ BLOCKSIZE;
    char blocks[MAXBLOCKS - blocks_used][BLOCKSIZE];

    fseek(fp, bytes_to_write, SEEK_CUR);
    
    // reading file blocks from the file
    //FILE DATA
    for (i = 0 ; i < MAXBLOCKS - blocks_used ; i++){
        if (fread(blocks[i], 1, BLOCKSIZE, fp) == 0) {
            fprintf(stderr, "Error: write failed on init\n");
            closefs(fp);
            exit(1);
        } 
    }

    closefs(fp);

    //Search for file
    int fentry_index = 0;
    int found = 0;
   
    for(i = 0; i < MAXFILES; i++){
        if (strcmp(files[i].name, simfilename) == 0){
            fentry_index = i;
            found = 1;
            break;
        }
    }

    // Output error if file does not exist
    if (found == 0) {
        fprintf(stderr, "Error: File does not exist\n");
        exit(1);
    }

    //change name and size attribute to empty
    strcpy(files[fentry_index].name, "\0");

    files[fentry_index].size = 0;



     // look for used blocks
    if (files[fentry_index].firstblock != -1) {
        blocks_index = 0;
        used_blocks[blocks_index] = files[fentry_index].firstblock ;
        blocks_index += 1;
        findusedblocks(used_blocks, files[fentry_index].firstblock, fnodes);
    }
    // change firstblock to -1
    files[fentry_index].firstblock = -1;
   

   // change fnodes to unused
    for (i = 0; i < blocks_index ; i++){
        fnodes[used_blocks[i]].blockindex = -used_blocks[i];
        fnodes[used_blocks[i]].nextblock = -1;
    }

    //overwriting the tile with zeros

    for(i = 0 ; i < blocks_index ; i++){
        for(int m = 0 ; m < BLOCKSIZE ; m++){
            blocks[used_blocks[i]- blocks_used][m] = 0;
        }
    }


    // write updated fentries and fnodes in the smfs

    fp = openfs(filename, "w");

    if(fwrite(files, sizeof(fentry), MAXFILES, fp) < MAXFILES) {
        fprintf(stderr, "Error: write failed on init\n");
        closefs(fp);
        exit(1);
    }

    if(fwrite(fnodes, sizeof(fnode), MAXBLOCKS, fp) < MAXBLOCKS) {
        fprintf(stderr, "Error: write failed on init\n");
        closefs(fp);
        exit(1);
    }

    fseek(fp, bytes_to_write, SEEK_CUR);

    //writing file blocks to the file
    //FILE DATA
    for (i = 0 ; i < MAXBLOCKS - blocks_used  ; i++){
        if (fwrite(blocks[i], BLOCKSIZE, 1, fp) < 1) {
            fprintf(stderr, "Error: write failed on init\n");
            closefs(fp);
            exit(1);
        } 
    }
    closefs(fp);


   
}

//READFILE

void readfile(char *filename, char *simfilename, int start, int length) {
    fentry files[MAXFILES];
    fnode fnodes[MAXBLOCKS];
    int i;

    // output error is file name exceeds 11 characters
    if (strlen(simfilename) > 11) {
        fprintf(stderr, "Error: not enough space for new file\n");
        exit(1);
    }
    
    //read fentries and fnodes into files and fnodes arrays, respectively.

    FILE *fp ;
    fp = openfs(filename, "r");

    if ((fread(files, sizeof(fentry), MAXFILES, fp)) == 0) {
        fprintf(stderr, "Error: could not read file entries\n");
        closefs(fp);
        exit(1);
    }

    if ((fread(fnodes, sizeof(fnode), MAXBLOCKS, fp)) == 0) {
        fprintf(stderr, "Error: could not read fnodes\n");
        closefs(fp);
        exit(1);
    }

    //Number of blocks used
    int bytes_used = sizeof(fentry) * MAXFILES + sizeof(fnode) * MAXBLOCKS;
    int bytes_to_write = (BLOCKSIZE - (bytes_used % BLOCKSIZE)) % BLOCKSIZE;
    int blocks_used = (bytes_used + bytes_to_write)/ BLOCKSIZE;
    char blocks[MAXBLOCKS - blocks_used][BLOCKSIZE];

    fseek(fp, bytes_to_write, SEEK_CUR);
    
    // reading file blocks from the file
    //FILE DATA
    for (i = 0 ; i < MAXBLOCKS - blocks_used ; i++){
        if (fread(blocks[i], 1, BLOCKSIZE, fp) == 0) {
            fprintf(stderr, "Error: write failed on init\n");
            closefs(fp);
            exit(1);
        } 
    }

    closefs(fp);


    //Search for file
    int fentry_index = 0;
    int found = 0;
   
    for(i = 0; i < MAXFILES; i++){
        if (strcmp(files[i].name, simfilename) == 0){
            fentry_index = i;
            found = 1;
            break;
        }
    }

    // Output error if file does not exist
    if (found == 0) {
        fprintf(stderr, "Error: File does not exist\n");
        exit(1);
    }


    //ERRORS

    if (files[fentry_index].size < start) {
        fprintf(stderr, "Error: File size is smaller than starting offset \n");
        exit(1);
    }

    if (start < 0 ) {
        fprintf(stderr, "Error:start cannot be a negatuve value \n");
        exit(1);
    }
    if (length <= 0) {
        fprintf(stderr, "Error:length cannot be zero or a negative value \n");
        exit(1);
    }

    if (files[fentry_index].size  < start + length) {
        fprintf(stderr, "Error: File size is smaller than length of data given \n");
        exit(1);
    }

    //emptying used blocks array
     for (i =0; i < blocks_index ; i++){
        used_blocks[i] = '\0';
    }



    //used blocks
    if (files[fentry_index].firstblock != -1) {
        blocks_index = 0;
        used_blocks[blocks_index] = files[fentry_index].firstblock ;
        blocks_index += 1;
        findusedblocks(used_blocks, files[fentry_index].firstblock, fnodes);

    }

    //starting block
    int starting_block = used_blocks[(start / BLOCKSIZE)] ;
    int ending_block = used_blocks[(start + length) / BLOCKSIZE] ;
    int ending_index = (start + length) % BLOCKSIZE ;

    
    i = start / BLOCKSIZE;
    if (start == 0) {
        while (i != (start + length) / BLOCKSIZE) {
            for (int m = 0 ;  m < BLOCKSIZE ; m++){
                printf("%c", blocks[used_blocks[i] - blocks_used][m]);
            }
            i += 1;
        }

        for (int m = 0 ;  m < ending_index ; m++){
            printf("%c", blocks[ending_block - blocks_used][m]);
            }
        printf("\n");
    }
    else {
        if (starting_block == ending_block) {
            for (int m = start % BLOCKSIZE ;  m < ending_index ; m++){
                printf("%c", blocks[used_blocks[i] - blocks_used][m]);
            }
            printf("\n");  
        }
        else {
            for (int m = start % BLOCKSIZE ;  m < BLOCKSIZE ; m++){
                printf("%c", blocks[used_blocks[i] - blocks_used][m]);
            }
            i += 1;
            while (i != (start + length) / BLOCKSIZE) {
                for (int m = 0 ;  m < BLOCKSIZE ; m++){
                    printf("%c", blocks[used_blocks[i] - blocks_used][m]);
                }
                i += 1;
            }

            for (int m = 0 ;  m < ending_index ; m++){
                printf("%c", blocks[ending_block - blocks_used][m]);
            }
            printf("\n");

            }

        }

}


//WRITEFILE
void writefile(char *filename, char *simfilename, int start, int length) {

    fentry files[MAXFILES];
    fnode fnodes[MAXBLOCKS];
    int i;

    // output error is file name exceeds 11 characters
    if (strlen(simfilename) > 11) {
        fprintf(stderr, "Error: not enough space for new file\n");
        exit(1);
    }
    
    //read fentries and fnodes into files and fnodes arrays, respectively.

    FILE *fp ;
    fp = openfs(filename, "r");

    if ((fread(files, sizeof(fentry), MAXFILES, fp)) == 0) {
        fprintf(stderr, "Error: could not read file entries\n");
        closefs(fp);
        exit(1);
    }

    if ((fread(fnodes, sizeof(fnode), MAXBLOCKS, fp)) == 0) {
        fprintf(stderr, "Error: could not read fnodes\n");
        closefs(fp);
        exit(1);
    }

    //Number of blocks used
    int bytes_used = sizeof(fentry) * MAXFILES + sizeof(fnode) * MAXBLOCKS;
    int bytes_to_write = (BLOCKSIZE - (bytes_used % BLOCKSIZE)) % BLOCKSIZE;
    int blocks_used = (bytes_used + bytes_to_write)/ BLOCKSIZE;
    char blocks[MAXBLOCKS - blocks_used][BLOCKSIZE];

    fseek(fp, bytes_to_write, SEEK_CUR);
    
    // reading file blocks from the file
    //FILE DATA
    for (i = 0 ; i < MAXBLOCKS - blocks_used ; i++){
        if (fread(blocks[i], 1, BLOCKSIZE, fp) == 0) {
            fprintf(stderr, "Error: write failed on init\n");
            closefs(fp);
            exit(1);
        } 
    }

    closefs(fp);


    //Search for file
    int fentry_index = 0;
    int found = 0;
   
    for(i = 0; i < MAXFILES; i++){
        if (strcmp(files[i].name, simfilename) == 0){
            fentry_index = i;
            found = 1;
            break;
        }
    }

    // Output error if file does not exist
    if (found == 0) {
        fprintf(stderr, "Error: File does not exist\n");
        exit(1);
    }

    //Output error is size of file is smaller than offset

    if (files[fentry_index].size < start) {
        fprintf(stderr, "Error: File size is smaller than the offset\n");
        exit(1);
    }
    
    char s[length+1];

    if (fread(s, sizeof(char), length, stdin) < length) {
         fprintf(stderr, "Error: Length of input is too small\n");
    }

    //emptying used blocks array
     for (i =0; i < blocks_index ; i++){
        used_blocks[i] = '\0';
    }
    for (i =0; i < blocks_index ; i++){
        printf("%d",used_blocks[i]);
    }



    //used blocks
    if (files[fentry_index].firstblock != -1) {
        blocks_index = 0;
        used_blocks[blocks_index] = files[fentry_index].firstblock ;
        blocks_index += 1;
        findusedblocks(used_blocks, files[fentry_index].firstblock, fnodes);

    }
    

    //no block in use
    if (blocks_index == 0){
    for (i = 0 ; i < MAXBLOCKS ; i++){
        if (fnodes[i].blockindex < 0){
            used_blocks[0] = i;
            fnodes[i].blockindex = i;
            files[fentry_index].firstblock = i;
            blocks_index = 1;
            break;
        }
    }
    }

    if (blocks_index == 0){
        fprintf(stderr, "Error: no space available to write\n");
    }


    // calculating where offset lies
    int starting_block = used_blocks[(start / BLOCKSIZE)] ;
    int ending_index = (start + length ) % BLOCKSIZE ;
    int starting_index = start % BLOCKSIZE;

    //index of used blocks
    int starting_block_index = start / BLOCKSIZE;
    int ending_block_index = (start + length - 1) / BLOCKSIZE ;

    

    if (ending_block_index >= blocks_index){

        //number of blocks required
        int blocks_required = ending_block_index - (blocks_index -1);
        int found = 0;
        int free_blocks[blocks_required];
   
        for(i = 0; i < MAXBLOCKS; i++){
            if (fnodes[i].blockindex < 0 ){
                free_blocks[found] = i ;
                found += 1;
                
            }
            if (found == blocks_required){
                break;
             } 
            }

        // Output error if free space not avaialable
        if (found < blocks_required) {
            fprintf(stderr, "Error: space not available \n");
            exit(1);
        }
        else {
            int m = 0;
            for (i = blocks_index ; i < blocks_index + blocks_required ; i++ ){
                used_blocks[i] = free_blocks[m];
                m += 1;
            }
            blocks_index += blocks_required;
            
        }
        
    }
    
    int ending_block = used_blocks[(start + length - 1) / BLOCKSIZE] ;
    
    //data can go in one block
    
    if (starting_block_index == ending_block_index) {
        if (ending_index == 0){
            ending_index = BLOCKSIZE;
        }

        int m = 0;
        int i;
        for (i = starting_index; i < ending_index ; i ++){
            blocks[starting_block - blocks_used][i] = s[m];
            m +=1 ;
    }
    }
    // data requires more blocks
    else {
        int m = 0;
        int starting_index = start % BLOCKSIZE;
        //first block
        for (i = starting_index; i < BLOCKSIZE ; i ++){
            blocks[starting_block - blocks_used][i] = s[m];
            m +=1 ;
        }
        int block_num = (start / BLOCKSIZE) + 1;
        
        //mid blocks
        while (block_num != (start + length - 1) / BLOCKSIZE) {
            for (i = 0; i < BLOCKSIZE ; i ++){
            blocks[used_blocks[block_num]][i] = s[m];
            block_num += 1;
            m += 1;

            }
        }
        //end block
        for (i = 0; i < ending_index+1; i ++){
            blocks[ending_block - blocks_used][i] = s[m];
            m +=1 ;
        }
        }
    

    // correct block and nextblock values
    for (i = 0 ; i < blocks_index - 1 ; i++){
        fnodes[used_blocks[i]].blockindex = used_blocks[i];
        fnodes[used_blocks[i]].nextblock = used_blocks[i+1];
    }
    //last block
    fnodes[used_blocks[i]].blockindex = used_blocks[i];


    //increase size
    if (start + length > files[fentry_index].size){
        files[fentry_index].size += length ;
    }


    fp = openfs(filename, "w");

    if(fwrite(files, sizeof(fentry), MAXFILES, fp) < MAXFILES) {
        fprintf(stderr, "Error: write failed on init\n");
        closefs(fp);
        exit(1);
    }

    if(fwrite(fnodes, sizeof(fnode), MAXBLOCKS, fp) < MAXBLOCKS) {
        fprintf(stderr, "Error: write failed on init\n");
        closefs(fp);
        exit(1);
    }


    fseek(fp, bytes_to_write, SEEK_CUR);

    //writing file blocks to the file
    //FILE DATA
    for (i = 0 ; i < MAXBLOCKS - blocks_used  ; i++){
        if (fwrite(blocks[i], BLOCKSIZE, 1, fp) < 1) {
            fprintf(stderr, "Error: write failed on init\n");
            closefs(fp);
            exit(1);
        } 
    }
    closefs(fp);
    


}
