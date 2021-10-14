#include <stdio.h>
#include "simfstypes.h"

/* File system operations */
void printfs(char *);
void initfs(char *);
void createfile(char *, char *simfilename);
void deletefile(char *filename, char *simfilename);
void readfile(char *filename, char *simfilename, int start, int length);
void writefile(char *filename, char *simfilename, int start, int length);
void findusedblocks(int *, int , fnode *);


/* Internal functions */
FILE *openfs(char *filename, char *mode);
void closefs(FILE *fp);
