#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/wait.h>

int create_process_and_run(char *command)
{
    // In a real implementation, you would use fork/exec or system() to run the command.
    // Here, we'll just print the command and return a dummy status.
    printf("Running command: %s\n", command);

    // trying fork
    int status = fork();
    if (status < 0)
    {
        printf("error");
    }
    else if (status == 0)
    {
        printf("child\n");
    }
    else
    {
        printf("parent\n");
    }
    return 0;
    // return status;
}

int launch(char *command)
{
    int status;
    status = create_process_and_run(command);
    return status;
}

void shell_loop()
{
    int status;
    do
    {
        printf("iiitd@possum:~$ ");
        char *command = NULL;
        size_t bufsize = 0;
        getline(&command, &bufsize, stdin);
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

        // status = launch(command);
        char *usr_cmd = args[0];
        char *path = (char*)malloc(256);
        snprintf(path, 256, "/%s/%s", "usr/bin",usr_cmd);
        // char *path = "/usr/bin/ls";
        char *user = getenv("USER");
        char *relative_path = (char*)malloc(7+strlen(user));
        snprintf(relative_path, 7+strlen(user), "/%s/%s", "home",user);
        char *args_[] = {path, relative_path, NULL};
        execv(path,args_);
        break;
    } while (status);
}

int main()
{
    shell_loop();
    return 0;
}