/*
 * main.c
 *
 * A simple program to illustrate the use of the GNU Readline library
 */

#include <unistd.h>
#include <string.h> 
 
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
void ExecuteNonInternal(char *input)
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


char* GetCurrentDirectory()
{
    char directory[2048];
    getcwd(directory, sizeof(char)*256);
    char suffix = '>';
    strncat(directory,&suffix,1);
    char *d = directory;
    return d;
}

void ChangeCWD(char* path)
{
    int return_value = chdir(path);
    if (return_value != 0){
        printf("Error: Path Not Found\n");
    }
}

int main ( void )
{
	for (;;)
	{
		char *cmd = readline(GetCurrentDirectory());
        
        char *token = strtok(cmd, " ");
        if (strcmp(token,"cd") == 0){
            //Second token will be path
            token = strtok(NULL, " ");
            ChangeCWD(token);
        }
        //Exectue will be default behaviour 
        ExecuteNonInternal(cmd);
		free (cmd);
	}	
}