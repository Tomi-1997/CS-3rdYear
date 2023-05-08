CC = gcc -Wall -g -pthread

.PHONY: all

all: prog

call: clean all

prog:	main.c
	$(CC) main.c queue.c ./libCodec.so -o prog 

tester: single.c
	$(CC) single.c ./libCodec.so -o tester 

.PHONY: all clean call
clean:
	-rm prog tester
