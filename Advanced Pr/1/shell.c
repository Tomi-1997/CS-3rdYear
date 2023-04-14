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
#define VAR_MAX 64

/* State control */
enum states 
{
    NEUTRAL,
    IF,
    THEN, 
    ELSE, 
    FI
};

struct local_var_
{
  char name[64];
  char value[64];
} typedef local_var;

char* token;
char* outfile;

pid_t pid;
char* argv[10][10];
char command[CMD_SIZE];
local_var variables[VAR_MAX];
char history[REMEMBER_SIZE][CMD_SIZE];
char prompt[PROMPT_LEN] = "hello:";

int inp, redirect, redirect_app, curr_state, if_status,
err_redirect, retid, status, cmds_count, var_table_size, i, fd, amper;

int bak, new; // close and re-open stdout

/*If block is executed line by line, the output will be stored in temporary files and will be removed after the block is finished*/
char std_fname[] = "delme";
char if_fname[] = "delme2";

int close_stdout(char* target)
{
    /*Closes stdout and redirects it to a target folder. */
    fflush(stdout);
    bak = dup(1);
    new = open(target, O_WRONLY | O_CREAT, 0660);
    dup2(new, 1);
    close(new);
    return 0;
}


int open_stdout()
{
    /*Closes the last folder the stdout was redirected into, reopens stdout. */
    fflush(stdout);
    dup2(bak, 1);
    close(bak);
    return 0;
}


int print_file(char* target)
{
    /*Prints content of a target file*/
    int c;
    FILE *file;
    file = fopen(target, "r");
    if (file) 
    {
        while ((c = getc(file)) != EOF)
            putchar(c);
        fclose(file);
    }
}


int file_to_bool(char* target)
{
    /*Opens file, if a file is empty or has zero values, returns 0*/
    int c;
    FILE *file;
    file = fopen(target, "r");
    int flag = 0;
    if (file) 
    {
        while ((c = getc(file)) != EOF)
            flag = flag || c;
        fclose(file);
    }
    return flag;
}


int next_state()
{
    switch (curr_state)
    {
        case NEUTRAL:
            curr_state = IF;
            close_stdout("/dev/null"); // Ignore parent output (prompt)
            break;
        case IF:
            curr_state = THEN;
            break;
        case THEN:
            curr_state = ELSE;
            break;
        case ELSE:
            open_stdout();              // Finished if block, print output from executed commands
            print_file(std_fname);    // and open stdout to continue printing prompts and output
            remove(std_fname);
            curr_state = NEUTRAL;
            break;
    }
    return 0;
}


int change_prompt(char* prompt, char* newp)
{
	int len = strlen(newp);
	
	int i = 0;
    /* Overwrite previous prompt until new prompt is over or stop at the middle if it's too long */
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


char* get_val(int index)
{
    return variables[index].value;
}


int find_var(local_var* table, char* target)
{
    for (int i = 0; i < var_table_size; i++)
    {
        // Found variable with the same name, return the index
        if (strcmp(table[i].name, target) == 0)
            {
                return i;
            }
    }
    return -1;
}


int swap_variables(char** args)
{
    /*For each command, swap $var to the value of var, if it exists, e.g
    read cmd flag
    ls -l
    and then run $flag $cmd
    will execute ls -l
    */
    int i = 0;
    while (args[i] != NULL)
    {
        /* Starts with $ sign? */
        if (args[i][0] == '$')
        {
            /* Is there a variable with the same name? */
            int vari = find_var(variables, ++args[i]); // call find_var while ignoring $ by incrementing the pointer.

            /* If so, replace current token with the value */
            if (vari != -1)
                strcpy(args[i], get_val(vari));
        }
        i++;
    }
    return 0;
}

int add_var(local_var* table, char* var, char* val)
{   
    /*Add a variable to variable table, from read x y cmd, or $x = val*/
    if (var == NULL || val == NULL || var_table_size >= VAR_MAX)
        return 0;

    int vari = find_var(table, var);
    /* If it's not in the table, add it and increase the current size */
    if (vari == -1)
    {
        vari = var_table_size;
        var_table_size++;
    }
    strcpy(table[vari].name, var);
    strcpy(table[vari].value, val);
    return 0;
}


int starts_with_if(char* cmd)
{
    if (strlen(cmd) < strlen("if "))
        return 0;
    
    return cmd[0] == 'i' && cmd[1] == 'f' && cmd[2] == ' ';
}


int get_user_cmd(int history_index)
{
    memset(command, '\0', sizeof(command));
    char* ret_val;
    /* History index is -1, new command incoming */
    if (history_index == -1)
    {
        printf("%s ", prompt);
        ret_val = fgets(command, CMD_SIZE, stdin);
    }
    else
    {
        /* History index is not -1, append incoming input into an existing command */
        strcpy(command, history[history_index]);
        printf("%s %s", prompt, command);
        char temp_command[CMD_SIZE];

        /* Get additional flags or more commands, append to current one showing on screen */
        ret_val = fgets(temp_command, CMD_SIZE, stdin);
        strncat(command, temp_command, strlen(temp_command));
    }

    if (ret_val == NULL) // detect ctrl+d
        exit(0);

    int len = strlen(command) - 1;
    command[len] = '\0';

    if (len == 0)
    {
        argv[0][0] = NULL;
        return 0;
    }

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

            return get_user_cmd(history_index);
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
    }

    int prefix = 0;
    /* check if command starts with if, switch state and update prefix to ignore 'if ' */
    if (starts_with_if(command) && curr_state == NEUTRAL)
    {
        if_status = -1;         // reset status of if command
        prefix = strlen("if ");
        next_state();           // adv state to 'IF'
    }

    /*then, if or else - switch state and skip*/
    else if ((curr_state == IF && strcmp(command, "then") == 0 ) || 
               (curr_state == THEN && strcmp(command, "else") == 0) ||
               (curr_state == ELSE && strcmp(command, "fi") == 0) )
    {
        next_state();
        argv[0][0] = NULL; 
        return 0;
    }
    /* parse command line */
    i = 0;
    cmds_count = 0;
    token = strtok (command + prefix," ");
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


int is_prompt_cmd(char** arguments)
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
    amper = 0;                // &
    redirect = 0;              // > 
    redirect_app = 0;       // >>
    err_redirect = 0;        // 2>
    inp = 0;                    // <

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

    if (strcmp(argv[cmds_count][out_i - 1], "<") == 0)
    {
        inp = 1;
    }

    if (redirect || err_redirect || amper || inp)
        argv[cmds_count][out_i - 1] = NULL; 

    outfile = argv[cmds_count][out_i];
    return 0;
}


int change_dir(char* target)
{
    return chdir(target);
}


int shell_read_cmd(char** args)
{
    /* Read each variable from stdin, attach to var names in current command, respectively */
    char** temp_pointer = args;
    temp_pointer++; // skip first word ('read')
    char buff[CMD_SIZE];
    fgets(buff, CMD_SIZE, stdin);
    buff[strlen(buff) - 1] = '\0';

    token = strtok (buff," ");

    /* Terminates once out of variables, or too many variables */
    while (token != NULL && *temp_pointer != NULL)
    {
        add_var(variables, *temp_pointer, token);
        token = strtok (NULL, " ");
        temp_pointer++;
    }
    return 0;
}


int exec_shell(char** bash_args)
{
    int len = strlen(bash_args[0]);

    if (strcmp(bash_args[0], "cd") == 0)
        return change_dir(bash_args[1]);

    if (is_prompt_cmd(bash_args))
        return change_prompt(prompt, bash_args[2]);

    if (len > 1 && bash_args[0][0] == '$')
        return add_var(variables, ++bash_args[0], bash_args[2]); // ++ to ignore $

    if (strcmp(bash_args[0], "read") == 0)
        return shell_read_cmd(bash_args);
    return 1;
}


int not_in(char val, char* target)
{
    for (int i = 0; i < strlen(target); i++)
    {
        if (target[i] == val)
            return 0;
    }
    return 1;
}


int is_shell_cmd(char** cmd)
{
    if (strlen(cmd[0]) < 2)
        return 1;

    if (strcmp(cmd[0], "cd") == 0)
        return 0;

    if (strcmp(cmd[0], "prompt") == 0)
        return 0;

    if (strcmp(cmd[0], "read") == 0)
        return 0;

    if (cmd[0][0] == '$' && cmd [1] != NULL && cmd[1][0] == '=')
        return 0;

    /* Not shell command, swap $vars with values from table */
    for (int j = 0; j < cmds_count + 1; j++)
        swap_variables(argv[j]);

    return 1;
}


int state_good()
{
    /* States to run a command in:
    -Then line and command returns non-zero
    -Else line and the output is zero
    -Neutral state, just run everything*/
    int good_else = (if_status == 0 && curr_state == ELSE);
    int good_then = (if_status != 0 && curr_state == THEN);
    return curr_state == NEUTRAL || good_else || good_then || curr_state == IF;
}


void sigintHandler()
{
    fflush(stdout);
}


int main() 
{    
    /* Initialize local status variable ($?)*/
    strcpy(variables[0].name, "?");
    strcpy(variables[0].value, "0");
    var_table_size = 1;

    /* Initialize if control to neutral*/
    curr_state = NEUTRAL;
    if_status = -1;

    /* Ignore ctrl C */
    signal(SIGINT, sigintHandler);

    while (1)
    {
        get_user_cmd(-1);            // Parse commands into argv[], or reload from history.
        if (!state_good())
            {
                continue;
            }
    
        if (argv[0][0] == NULL)        // If empty- skip
            continue;

        if (strcmp(argv[0][0], "quit") == 0)
        {
            return 0;
        }

        if (is_shell_cmd(argv[0]) == 0)
        {
            exec_shell(argv[0]);
            continue;
        }

        /* for commands not part of the shell command language */ 
        update_io();
        pid = fork();
        if (pid == 0) 
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

            /* input redirect? */
            if (inp)
            {
                fd = open(outfile, O_RDONLY, 0);
                close(STDIN_FILENO);
                dup(fd);
                close(fd);
            }

            /* if statement, let parent convert output to boolean and ignore it */
            if (curr_state == IF)
                close_stdout(if_fname);

            /* inside then/else, whatever is running, output to a temporary file, to be read later */
            if (curr_state == THEN || curr_state == ELSE)
                close_stdout(std_fname);
            
            execute_cmds();
            return 0;
        }

        /* parent continues here */
        if (amper == 0)
            {
                retid = waitpid(pid, &status, 0);            // Status of last cmd
                char stat[2];                     // Convert to array of 2 chars, (status and '\0')
                snprintf(stat, 2, "%d", status);
                strcpy(variables[0].value, stat); // Save last command status

                if (curr_state == IF)
                   {
                        if_status = file_to_bool(if_fname);
                        remove(if_fname);
                    }
            }
    }

    return 0;
}
