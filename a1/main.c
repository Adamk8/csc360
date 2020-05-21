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
        printf("%s: command not found\n", arguments[0]);
    }
    //Wait for child process to finish
    else { 
        waitpid(child_pid,&childStatus,WUNTRACED);
    }
}


const char* GetCurrentDirectory()
{
    char *directory = malloc(sizeof(char)*256);
    getcwd(directory, sizeof(char)*256);
    char suffix = '>';
    strncat(directory,&suffix,1);
    return directory;
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
        const char *dir = GetCurrentDirectory();
		char *cmd = readline(dir);
        char *cmd_copy = malloc(sizeof(cmd));
        strcpy(cmd_copy,cmd);
        char *token = strtok(cmd_copy, " ");
        if (strcmp(token,"cd") == 0){
            //Second token will be path
            token = strtok(NULL, " ");
            ChangeCWD(token);
        } 
        //Exectue will be default behaviour         
        else {
            ExecuteNonInternal(cmd);
        }
        free(dir);
        free(cmd_copy);
		free(cmd);
	}	
}