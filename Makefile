all: D

D: a.out
	gcc -o D a.out

a.out: filesys.c shell.c filesys.h
	gcc -c shell.c filesys.c filesys.h
