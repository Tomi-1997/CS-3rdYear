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
#define REMEMBER_SIZE 100

char* token;
char* outfile;
char* argv[10][10];
char command[CMD_SIZE];
char history[REMEMBER_SIZE][CMD_SIZE];
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

int remember_command()
{
    // Move previous commands up
    for (int i = REMEMBER_SIZE - 1; i > 0; i--)
    {
        strcpy(history[i], history[i - 1]);
    }

    // First entry is the newest command.
    strcpy(history[0], command);
}

int temp()
{
    FILE *fptr;
   // use appropriate location if you are using MacOS or Linux
   fptr = fopen("temp","a+"); // append
   if(fptr == NULL)
   {
      printf("Error!");   
      exit(1);             
   }
   fprintf(fptr,"%s\n",history[0]);
   fclose(fptr);
   return 0;
}

int get_command(int history_index)
{
    /* History index is -1, new command incoming */
    if (history_index == -1)
    {
        printf("%s ", prompt);
        memset(command, '\0', sizeof(command));
        fgets(command, CMD_SIZE, stdin);
    }

    else
    {
        /* History index is not -1, append incoming input into an existing command */
        strcpy(command, history[history_index]);
        printf("%s %s", prompt, command);
        char temp_command[CMD_SIZE];
        fgets(temp_command, CMD_SIZE, stdin);
        strncat(command, temp_command, strlen(temp_command));
    }

    int len = strlen(command) - 1;
    command[len] = '\0';

    // Is last character arrow?
    if (command[len - 3] == '\033')
    {
            switch (command[len - 1])
            {
                case 'A':                                   // Up arrow
                    if (history_index < REMEMBER_SIZE - 1)  // Make sure within limits
                    {
                        history_index++;
                    }
                    break;
                case 'B': // Down arrow
                    if (history_index > -1)                 // Can go back to previous commands
                    {
                        history_index--;
                    }
                    else                                    // Can't go further down, reached bottom of history list 
                    {
                    }
            }

            return get_command(history_index);
    }

    // Is !! command?
    if (strcmp(command, "!!") == 0)
    {
        strcpy(command, history[0]);
    }
    
    // Is new command?
    if (!(strcmp(command, history[0]) == 0))
    {
        remember_command();
        temp();
    }

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
        return 0;      // Is command long enough to have an & command

    int out_i = i - 1; // Temporary i index to hold filename pos

    if (strcmp(argv[cmds_count][i - 1], "&") == 0)
    {
        amper = 1;
        argv[cmds_count][i - 1] = NULL;
        out_i--; // If & is present, then filename pos is second to last
    }

    if (i < 3)      // Is command long enough to have io commands
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

int change_dir(char* target)
{
    return chdir(target);
}

int exec_shell(char** bash_args)
{
    int res = 1;

    if (strcmp(bash_args[0], "cd") == 0)
        res = change_dir(bash_args[1]);

    return res;
}

int shell_command(char* cmd)
{
    int ans = 1;

    if (strcmp(cmd, "cd") == 0)
        ans = 0;

    return ans;
}

int main() 
{    
    while (1)
    {
        get_command(-1);            // Parse commands into argv[], or reload from history.

        if (argv[0] == NULL)        // If empty- skip
            continue;

        if (strcmp(argv[0][0], "quit") == 0)
            return 0;

        if (prompt_cmd(argv[0]))       // If prompt command - validate and change
        {
            change_prompt(prompt, argv[0][2]);
            continue;
        }

        if (shell_command(argv[0][0]) == 0)
        {
            exec_shell(argv[0]);
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
