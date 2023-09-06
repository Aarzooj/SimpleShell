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
    // In a real implementation, you would use fork/exec or system() to run the command.
    // Here, we'll just print the command and return a dummy status.
    // printf("Running command: %s\n", command);
    // char *path = (char *)malloc(256);
    // printf("%s\n",usr_cmd);
    // trying fork
    int status = fork();
    if (status < 0)
    {
        printf("error");
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
        execvp(args[0],args);
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
    // printf("%s\n",command);
    status = create_process_and_run(args);
    return status;
}

char *read_user_input()
{
    char *input = (char *)malloc(256);
    size_t buffsize = 0;
    getline(&input, &buffsize, stdin);
    return input;
}

void shell_loop()
{
    int status;
    do
    {
        printf("iiitd@possum:~$ ");
        char *command = read_user_input();
        command = strtok(command,"\n");
        // printf("%s\n",command);
        char **args = (char **)malloc(256 * sizeof(char *));
        int countArgs = 0;

        char *split = strtok(command, " ");
        while (split != NULL)
        {
            args[countArgs++] = split;
            split = strtok(NULL, " ");
        }
        // for (int i = 0; args[i] != NULL; i++)
        // {
        //     printf("%s\n", args[i]);
        // }

        status = launch(args);
        // char *usr_cmd = args[0];
        // break;
    } while (status);
}

int main()
{
    shell_loop();
    return 0;
}