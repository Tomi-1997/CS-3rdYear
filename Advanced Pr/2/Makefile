CC = gcc -Wall -g -pthread
LINK = sudo cp libCodec.so /usr/lib

.PHONY: all

all: prog

call: clean all

prog:	main.c
	$(CC) -o prog main.c queue.c -L. -l:libCodec.so 
	${LINK}

tester: single.c
	$(CC) -o tester single.c -L. -l:libCodec.so
	${LINK}

.PHONY: all clean call
clean:
	-rm prog tester
