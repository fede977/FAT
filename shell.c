#include <stdio.h>
#include <stdlib.h>
#include "filesys.h"

void D(){
    
    format();
    writedisk("virtualdiskD3_D1");

}

void C(){
    diskblock_t block;
    MyFILE * File = myfopen("testfile.txt", "w");

    char * alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXWZ";

    for(int i = 0; i<(4*BLOCKSIZE); i++){
        myfputc(alphabet[i%strlen(alphabet)], File);
    }

    myfputc(EOF,File);

    myfclose(File);

    writedisk("virtualdiskC3_C1");

    char fileChar;
    File = myfopen("testfile.txt", "w");
    FILE * returningFile = fopen("testfileC3_C1_copy.txt", "w");

    while(fileChar != EOF){
        fileChar = myfgetc(File);
        if (fileChar != EOF){
            fprintf(returningFile, "%c", fileChar);
            printf("%c", fileChar);
            
        }
    }
    fclose(returningFile);
}

void B(){
    char * path = "/myfirstdir/myseconddir/mythirddir";

    mymkdir(path);
    writedisk("virtualdiskB3_B1a");
    

    path = "/myfirstdir/myseconddir/testfile.txt";
    mymkdir(path);
    mylistdir("/myfirstdir/myseconddir/");
    
    writedisk("virtualdiskB3_B1b");
}

int main(int argc, char const *argv[]){
    D();
    C();
    B();
    return 0;
}
