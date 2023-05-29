CC = gcc -Wall -g -pthread

.PHONY: all

all: stree

call: clean all

stree:	stree.c
	$(CC) stree.c -o stree

.PHONY: all clean call
clean:
	-rm stree
