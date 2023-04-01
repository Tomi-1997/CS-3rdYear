#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>

#define PR_LEN 64

int change_prompt(char* prompt, char* newp)
{
	int len = strlen(newp);
	
	int i = 0;
	while (i < len && i < PR_LEN - 1)
	{
		prompt[i] = newp[i];
		i++;
	}
	
	prompt[i] = '\0';
	return 0;
}

int main() 
{
char command[1024];
char *token;
char *outfile;
int i, fd, amper, redirect, redirect_app, err_redirect, retid, status;
char *argv[10];

char prompt[PR_LEN] = "hello:";



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

    /* Is quit command */
    if (strcmp(argv[0], "quit") == 0)
	    return 0;

    /* Check for prompt change */
    if (strcmp(argv[0], "prompt") == 0)
    {
	    change_prompt(prompt, argv[2]);
	    continue;
    }
    
    /* Does command line end with & */ 
    if (! strcmp(argv[i - 1], "&")) {
        amper = 1;
        argv[i - 1] = NULL;
    }
    else 
        amper = 0; 

    if (!strcmp(argv[i - 2], ">") || !strcmp(argv[i - 2], ">>")) 
    {
	if (!strcmp(argv[i - 2], ">>"))
	{
		redirect_app = 1;
	}
	else
	{
		redirect_app = 0;
	}
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
	    if (redirect_app)
		    fd = open(outfile, O_RDWR | O_APPEND | O_CREAT);
	    else
            	fd = creat(outfile, 0660); 
            close (STDOUT_FILENO) ; 
            dup(fd); 
            close(fd); 
            /* stdout is now redirected */
        } 

	/* redirection of ERR? */
	if (err_redirect)
	{
		fd = creat(outfile, 0660);
		close(STDERR_FILENO);
		dup(fd);
		close(fd);
	}
        execvp(argv[0], argv);
    }

    /* parent continues here */
    if (amper == 0)
        retid = wait(&status);
}
return 0;
}
