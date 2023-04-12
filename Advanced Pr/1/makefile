GCC = gcc -Wall -g

all: prog

prog: shell.o
	$(CC) -o myshell shell.o

shell.o: shell.c
	$(CC) -c shell.c

clean:
	rm -f *.o myshell
