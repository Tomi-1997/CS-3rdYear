CC= gcc -Wall -g
LINK = sudo cp libCodec.so /usr/lib

.PHONY: all
all: task stdinExample

task: codec.h basic_main.c
	$(CC) -o encoder basic_main.c -L. -l:libCodec.so 
	${LINK}

stdinExample:	stdin_main.c
	$(CC) -o tester stdin_main.c -L. -l:libCodec.so 
	${LINK}
		
.PHONY: all clean
clean:
	-rm encoder tester 
