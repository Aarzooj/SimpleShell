#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>                           
#include <pthread.h>
#include <sys/wait.h>
#include <time.h>

#define INPUT_SiZE 256
#define HISTORY_SIZE 100

struct CommandParameter{
    char command[INPUT_SiZE];
    time_t start_time;
    time_t end_time;
    double duration;
    pid_t process_pid;
};

struct CommandHistory{
    struct CommandParameter record[HISTORY_SIZE];
    int historyCount;
};

struct CommandHistory history;

void displayHistory(){

    for(int i=0;i<history.historyCount;i++){
        struct CommandParameter record = history.record[i];
        printf("%s %d\n",record.command,record.process_pid);
        printf("%s %s %.2lf\n",ctime(&record.start_time),ctime(&record.end_time),record.duration);
        printf("--------------------------------\n");
    }

}

void display(){

    for(int i=0;i<history.historyCount;i++){
        printf("%d  %s\n",i+1,history.record[i].command);
    }

}


int create_process_and_run(char **args)
{
    // printf("Running command: %s\n", command);
    // char *path = (char *)malloc(256);
    // printf("%s\n",usr_cmd);
    int status = fork();
    if (status < 0)
    {
        printf("Error");
    }
    else if (status == 0)
    {
        // snprintf(path, 256, "/%s/%s", "usr/bin", usr_cmd);
        // // char *path = "/usr/bin/ls";
        // char *user = getenv("USER");
        // char *relative_path = (char *)malloc(7 + strlen(user));
        // snprintf(relative_path, 7 + strlen(user), "/%s/%s", "home", user);
        // char *args_[] = {path,"-a", relative_path, NULL};
        // execv(path, args_);

        int check = execvp(args[0],args);
        if (check == -1){
            printf("Error running execvp system call\n");
            return -1;
        }
        printf("child\n");
    }
    else
    {
        int child_status;
        wait(&child_status); // Wait for the child to complete
        if (WIFEXITED(child_status))
        {
            int exit_code = WEXITSTATUS(child_status);
            printf("Child process exited with status: %d\n", exit_code);
        }
        else
        {
            printf("Child process did not exit normally.\n");
        }
        printf("parent\n");
    }
    return status;
}

int launch(char **args)
{
    int status;
    status = create_process_and_run(args);
    return status;
}

char *read_user_input()
{
    char *input = (char *)malloc(INPUT_SiZE);
    size_t size = 0;
    int read = getline(&input, &size, stdin);
    if (read != -1){
        return input;    
    }
    else{
        perror("Error while reading line\n");
    }   
}

void shell_loop()
{
    int status;
    do
    {   char *user = getenv("USER");
        char host[256];
        int hostname  = gethostname(host,sizeof(host));
        printf("%s@%s~$ ",user,host);

        char *command = read_user_input();
        command = strtok(command,"\n");
        char *tmp = strdup(command);
        if (tmp == NULL) {
            perror("Error in strdup");
            exit(EXIT_FAILURE);
        }
 
        char **args = (char **)malloc(INPUT_SiZE * sizeof(char *));
        int count = 0;
        const char delim[2] = " ";

        char *token;
        token = strtok(command, delim);
        while (token != NULL)
        {
            args[count] = token;
            token = strtok(NULL, delim);
            count++;
        }

        if (strcmp(args[0],"history") == 0){
            if (history.historyCount > 0){
                displayHistory();
            }
            else{
                printf("No command in the history\n");
            }
        }
        else{

            strcpy(history.record[history.historyCount].command,tmp);
            history.record[history.historyCount].start_time = time(NULL);

            status = launch(args);

            history.record[history.historyCount].end_time = time(NULL);
            history.record[history.historyCount].process_pid = getpid();
            history.record[history.historyCount].duration = difftime(
                history.record[history.historyCount].end_time,
                history.record[history.historyCount].start_time
            );

            history.historyCount++;

        }
    } while (status);

}

int main()
{   
    history.historyCount = 0;
    shell_loop();
    printf("Exit\n");
    // displayTerminate();
    return 0;
}