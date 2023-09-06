#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>                           
#include <pthread.h>
#include <sys/wait.h>

int create_process_and_run(char **args)
{
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
            printf("Error running execvp system call");
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
    // return 0;
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
    char *input = (char *)malloc(256);
    size_t size = 0;
    int read = getline(&input, &size, stdin);
    if (read != -1){
        return input;    
    }
    else{
        perror("Error while reading line");
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

        char **args = (char **)malloc(256 * sizeof(char *));
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
        // for (int i = 0; args[i] != NULL; i++)
        // {
        //     printf("%s\n", args[i]);
        // }
        status = launch(args);
    } while (status);
}

int main()
{
    shell_loop();
    return 0;
}