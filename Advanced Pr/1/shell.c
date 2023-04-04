#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>

#define PROMPT_LEN 64
#define CMD_SIZE 1024

char* token;
char* outfile;
char* argv[10][10];
char command[CMD_SIZE];
char prompt[PROMPT_LEN] = "hello:";
int i, fd, amper, redirect, redirect_app, 
                err_redirect, retid, status, cmds_count;


int change_prompt(char* prompt, char* newp)
{
	int len = strlen(newp);
	
	int i = 0;
	while (i < len && i < PROMPT_LEN - 1)
	{
		prompt[i] = newp[i];
		i++;
	}
	
	prompt[i] = '\0';
	return 0;
}

int get_command()
{
    memset(command, '\0', sizeof(command));
    fgets(command, CMD_SIZE, stdin);
    command[strlen(command) - 1] = '\0';

    /* parse command line */
    i = 0;
    cmds_count = 0;
    token = strtok (command," ");
    while (token != NULL)
    {
        if (strcmp(token, "|") == 0) // Found |, new command incoming
        {
            argv[cmds_count][i] = NULL;
            cmds_count++;
            i = 0;
        }
        else // No |, same command
        {
            argv[cmds_count][i] = token;
            i++;
        }
        token = strtok (NULL, " ");
    }

    argv[cmds_count][i] = NULL;
    return 0;
}

int prompt_cmd(char** arguments)
{
    return (strcmp(arguments[0], "prompt")) == 0 && (strcmp(arguments[1], "=")) == 0 && i > 2;
}

int create_procces(int inp, int outp, int cmd_index)
{
    pid_t pid;
    if ((pid = fork()) == 0) // Child proccess does the command.
    {
        if (inp != 0)
        {
            dup2(inp, 0); // Stdin now refers to the given input fd.
            close(inp);
        }
        if (outp != 1)
        {
            dup2(outp, 1); // Stdout now refers to the given output fd.
            close(outp);
        }
        return execvp(argv[cmd_index][0], argv[cmd_index]); // Return command status
    }
    return pid;
}

int execute_cmds()
{
    int i;
    pid_t pid;
    int input, fd[2];

    input = 0; // First input is from std in.
    for (i = 0; i < cmds_count; i++) // Do all commands except the last, with the i+1 command getting input from the i command.
    {
        pipe(fd);
        create_procces(input, fd[1], i);
        close(fd[1]);
        input = fd[0];
    }

    if (input != 0) // Last command takes input from the i - 1 command. (Unless there is only one command)
        dup2(input, 0);

    return execvp(argv[i][0], argv[i]);
}

int update_io()
{
    amper = 0;
    redirect = 0;
    redirect_app = 0;
    err_redirect = 0;

    if (i < 2)
        return 0;      // Enough cmds have io or &

    int out_i = i - 1; // Temporary i index to hold filename pos

    if (strcmp(argv[cmds_count][i - 1], "&") == 0)
    {
        amper = 1;
        argv[cmds_count][i - 1] = NULL;
        out_i--; // If & is present, then filename pos is second to last
    }

    if (i < 3)      // Enough cmds to have io re-direction
        return 0;

    if (strcmp(argv[cmds_count][out_i - 1], ">") == 0)
    {
        redirect = 1;
    }

    if (strcmp(argv[cmds_count][out_i - 1], ">>") == 0)
    {
        redirect = 1;
        redirect_app = 1;
    }

    if (strcmp(argv[cmds_count][out_i - 1], "2>") == 0)
    {
        err_redirect = 1;
    }

    argv[cmds_count][out_i - 1] = NULL;
    outfile = argv[cmds_count][out_i];
    return 0;
}

int main() 
{    
    while (1)
    {
        printf("%s ", prompt);      // Print promot 
        get_command();              // Parse command into argv[]

        if (argv[0] == NULL)        // If empty- skip
            continue;

        if (strcmp(argv[0][0], "quit") == 0)
            return 0;

        if (prompt_cmd(argv[0]))       // If prompt command - validate and change
        {
            change_prompt(prompt, argv[0][2]);
            continue;
        }
        
        /* for commands not part of the shell command language */ 
        update_io();
        if (fork() == 0) 
        { 
            /* redirection of IO ? */
            if (redirect) 
            {
                if (redirect_app)                   // >>
                    fd = open(outfile, O_RDWR | O_APPEND | O_CREAT, 0660);
                else
                    fd = creat(outfile, 0660);      // >
                close (STDOUT_FILENO); 
                dup(fd); // Copies fd into the lowest fd available (which is stdout as we just closed it)
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
            
            execute_cmds();
        }

        /* parent continues here */
        if (amper == 0)
            retid = wait(&status);
    }
    return 0;
}
