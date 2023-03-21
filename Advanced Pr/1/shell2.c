#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>

int main() 
{
char command[1024];
char *token;
char *outfile;
int i, fd, amper, redirect, err_redirect, retid, status;
char *argv[10];
char* prompt = "hello:";

while (1)
{
    printf("%s ", prompt);
    fgets(command, 1024, stdin);
    command[strlen(command) - 1] = '\0';

    /* parse command line */
    i = 0;
    token = strtok (command," ");
    while (token != NULL)
    {
        argv[i] = token;
        token = strtok (NULL, " ");
        i++;
    }
    argv[i] = NULL;

    /* Is command empty */
    if (argv[0] == NULL)
        continue;

    /* Does command line end with & */ 
    if (! strcmp(argv[i - 1], "&")) {
        amper = 1;
        argv[i - 1] = NULL;
    }
    else 
        amper = 0; 

    if (! strcmp(argv[i - 2], ">")) 
    {
        redirect = 1;
        argv[i - 2] = NULL;
        outfile = argv[i - 1];
        }
    else 
        redirect = 0; 

    /*Redirect error to another place*/
    if (redirect == 0 && strcmp(argv[i - 2], "2>") == 0)
    {
	    err_redirect = 1;
	    argv[i - 2] = NULL;
	    outfile = argv[i - 1];
    }
    else
    {
	    err_redirect = 0;
    }

    /* for commands not part of the shell command language */ 

    if (fork() == 0) 
    { 
        /* redirection of IO ? */
        if (redirect) 
	{
            fd = creat(outfile, 0660); 
            close (STDOUT_FILENO) ; 
            dup(fd); 
            close(fd); 
            /* stdout is now redirected */
        } 

	/* redirection of ERR? */
	if (err_redirect)
	{
	}
        execvp(argv[0], argv);
    }
    /* parent continues here */
    if (amper == 0)
        retid = wait(&status);
}
}
