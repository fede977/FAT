.PHONY: shell

shell: filesys.c shell.c
	gcc shell.c filesys.c -o shell
