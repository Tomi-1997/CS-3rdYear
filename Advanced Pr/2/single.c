#include "codec.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
	    printf("usage: key < file \n");
	    printf("!! data more than 1024 char will be ignored !!\n");
	    return 0;
	}

	int key = atoi(argv[1]);

	char c;
	int counter = 0;
	int dest_size = 1024;
	char data[dest_size]; 
	

	while ((c = getchar()) != EOF)
	{
	  data[counter] = c;
	  counter++;

	  if (counter == 1024){
		encrypt(data,key);
		printf("%s",data);
		counter = 0;
	  }
	}
	
	if (counter > 0)
	{
		char lastData[counter];
		lastData[0] = '\0';
		strncat(lastData, data, counter);
		encrypt(lastData,key);
		printf("%s",lastData);
	}

	return 0;
}
