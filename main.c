/*
 * main.c
 *
 * A simple program to illustrate the use of the GNU Readline library
 */

#include <unistd.h>
 
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>


pid_t fork(void);
pid_t waitpid(pid_t pid, int *wstatus, int options);
int execvp(const char *file, char *const argv[]);

/*
This uses fork and execvp to perfrom basic terminal commands 
*/
void ExecuteBasic(char *input)
{

    char *token = strtok(input, " ");

    //Max 15 arguments +1 for the command
    const char *arguments[16];
    int arg_count = 0;
    //Used for waiting for child process 
    int childStatus;
    while( token != NULL ) {
        arguments[arg_count] = token;
        arg_count++;
        token = strtok(NULL, " ");
    }
    //Set last argument to NULL to end execvp
    arguments[arg_count] = NULL;
    pid_t child_pid = fork();
    if (child_pid == 0){
        execvp(arguments[0],arguments);
    }
    //Wait for child process to finish
    else { 
        waitpid(child_pid,&childStatus,WUNTRACED);
    }
}


int main ( void )
{
	for (;;)
	{
		char *cmd = readline ("shell>");
        // Exectue will be default behaviour 
        ExecuteBasic(cmd);
		free (cmd);
	}	
}