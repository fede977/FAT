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

    for(int i = 0; i<4*BLOCKSIZE; i++){
        myfputc(alphabet[i%strlen(alphabet)], File);
    }

    myfclose(File);

    writedisk("virtualdiskC3_C1");
}

int main(int argc, char const *argv[]){
    D();
    C();
    return 0;
}
