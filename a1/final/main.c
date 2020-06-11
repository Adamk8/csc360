/*
 * main.c
 *
 * A simple program to illustrate the use of the GNU Readline library
 */

#include <unistd.h>
#include <string.h> 
#include <signal.h>
 
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_PROCESSES 5

pid_t fork(void);
pid_t waitpid(pid_t pid, int *wstatus, int options);
pid_t wait();
int execvp(const char *file, char *const argv[]);

struct Process {
    int id;
    pid_t process_pid;
    char *name;
    char status;
};
struct Process bg_processes[MAX_PROCESSES];

int bg_count = 0;

/*
This uses fork and execvp to perfrom basic terminal commands 
*/
void ExecuteNonInternal(char *input, int bg)
{

    char *token = strtok(input, " ");
    //Skip bg token if bg
    if (bg) {
        token = strtok(NULL, " ");
    }

    //Max 15 arguments +1 for the command
    char *arguments[16];
    int arg_count = 0;
    //Used for waiting for child process 
    while( token != NULL ) {
        arguments[arg_count] = token;
        arg_count++;
        token = strtok(NULL, " ");
    }
    //Set last argument to NULL to end execvp
    if (bg){
        arguments[arg_count] = "&";
        arg_count++;
    }
    arguments[arg_count] = NULL;
    char *base_cmd = arguments[0];
    int child_status;
    pid_t child_pid = fork();
    if (child_pid == 0){
        if (bg && bg_count >= 5){
            printf("Max Background Processes running, Did not start: %s\n",arguments[0]);
            kill(getpid(),SIGKILL);
        } else {
            execvp(arguments[0],arguments);
            printf("%s: command not found\n", arguments[0]);
            kill(getpid(),SIGKILL);
        }
    }
    //Wait for child process to finish
    else { 
        usleep( 50000 );
        if (bg  && bg_count < 5){
            pid_t result = waitpid(child_pid, &child_status, WNOHANG);
            if (result != 0){
                bg_count--;
            }
            for (int i = 0; i < 5; i++){
                if (bg_processes[i].process_pid == 0){
                    bg_processes[i].process_pid = child_pid;
                    bg_processes[i].id = i;
                    bg_processes[i].name = malloc(sizeof(base_cmd));
                    strcpy(bg_processes[i].name,base_cmd);
                    bg_processes[i].status = 'R';
                    bg_count++;
                    break;
                }
            }
        }
        else {
            wait();
        }
    }
}

char* GetCurrentDirectory()
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

void ListBGProcesses() 
{
    int total = 0;
    for (int i = 0; i < 5; i++){
        if (bg_processes[i].process_pid > 0){
            printf("%d[%c]:  %s  PID: %d \n",bg_processes[i].id,bg_processes[i].status,bg_processes[i].name,bg_processes[i].process_pid);
            total++;
        }
    }
    printf("Total background processes: %d\n",total);
}

void KillBGProcess(char *input)
{
    char *token = strtok(input, " ");
    //Skip bg token if bg
    token = strtok(NULL, " ");
    int id = atoi(token);
    for (int i = 0; i < 5; i++){
        if (bg_processes[i].id == id){
            kill(bg_processes[i].process_pid, SIGKILL);
            printf("Killed background process: %d:  %s  PID: %d \n",bg_processes[i].id,bg_processes[i].name,bg_processes[i].process_pid);
            bg_processes[i].process_pid = 0; 
            bg_processes[bg_count].id = -1; 
            free(bg_processes[i].name);
            bg_count--;
            return;
        }
    }
    printf("Could not find process with ID: %d\n",id);
}

void StopBGProcess(char *input)
{
    char *token = strtok(input, " ");
    //Skip bg token if bg
    token = strtok(NULL, " ");
    int id = atoi(token);
    for (int i = 0; i < 5; i++){
        if (bg_processes[i].id == id){
            if (bg_processes[i].status == 'S'){
                printf("Error: Process already stopped\n");
            }
            else {
                kill(bg_processes[i].process_pid, SIGSTOP);
                bg_processes[i].status = 'S';
            }
            return;
        }
    }
    printf("Could not find process with ID: %d\n",id);
}

void ResumeBGProcess(char *input)
{
    char *token = strtok(input, " ");
    //Skip bg token if bg
    token = strtok(NULL, " ");
    int id = atoi(token);
    for (int i = 0; i < 5; i++){
        if (bg_processes[i].id == id){
            if (bg_processes[i].status == 'R'){
                printf("Error: Process already running\n");
            }
            else {
                kill(bg_processes[i].process_pid, SIGCONT);
                bg_processes[i].status = 'R';
            }
            return;
        }
    }
    printf("Could not find process with ID: %d\n",id);
}

int CheckBGStatus(){
    int status; 
    for (int i = 0; i < 5; i++){
            pid_t result = waitpid(bg_processes[i].process_pid, &status, WNOHANG);
            //if bg got invalid command set pid to 0 and skip
            if (result == -1){
                bg_processes[i].process_pid = 0;
                return 0;
            }
            else if (result != 0 && bg_processes[i].process_pid != 0){
                printf("Background process: %d: %s  PID: %d COMPLETED\n",bg_processes[i].id,bg_processes[i].name,bg_processes[i].process_pid);
                bg_processes[i].process_pid = 0;
                bg_count--;
                return 1;
            }
    }
    return 0;
}

int main ( void )
{
	for (;;)
	{
        char *dir = GetCurrentDirectory();
		char *cmd = readline(dir);
        char *cmd_copy = malloc(sizeof(cmd));
        strcpy(cmd_copy,cmd);
        char *token = strtok(cmd_copy, " ");
        if (strcmp(token,"cd") == 0){
            //Second token will be path
            token = strtok(NULL, " ");
            ChangeCWD(token);
        } 
        else if (strcmp(token,"bg") == 0){
            ExecuteNonInternal(cmd,1);    
        }
        else if (strcmp(token,"bglist") == 0){
            ListBGProcesses();
        }
        else if (strcmp(token,"bgkill") == 0){
            KillBGProcess(cmd);
        }
        else if (strcmp(token,"stop") == 0){
            StopBGProcess(cmd);
        }
        else if (strcmp(token,"start") == 0){
            ResumeBGProcess(cmd);
        }
        //Exectue will be default behaviour         
        else {
            ExecuteNonInternal(cmd,0);
        }
        CheckBGStatus();
        free(dir);
        free(cmd_copy);
		free(cmd);
	}	
}