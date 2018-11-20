#include <stdio.h>
#include <stdlib.h>
#include "filesys.h"


void B(){
    format();
    char * path = "/myfirstdir/myseconddir/mythirddir";

    mymkdir(path);
    writedisk("virtualdiskB3_B1a");

    path = "/myfirstdir/myseconddir/testfile.txt";
    mymkdir(path);

    writedisk("virtualdiskB3_B1b");
}

int main(int argc, char const *argv[]){
    B();
    return 0;
}