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

        int check = execvp(args[0], args);
        if (check == -1)
        {
            printf("Error running execvp system call\n");
            return -1;
        }
        // printf("child\n");
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
        // printf("parent\n");
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
    if (read != -1)
    {
        return input;
    }
    else
    {
        perror("Error while reading line\n");
    }
}

char *strip(char *string)
{
    char stripped[strlen(string) + 1];
    int len = 0;
    int flag;
    if (string[0] != ' ')
    {
        flag = 1;
    }
    else
    {
        flag = 0;
    }
    for (int i = 0; string[i] != '\0'; i++)
    {
        if (string[i] != ' ' && flag == 0)
        {
            stripped[len++] = string[i];
            flag = 1;
        }
        else if (flag == 1)
        {
            stripped[len++] = string[i];
        }
        else if (string[i] != ' ')
        {
            flag = 1;
        }
    }
    stripped[len] = '\0';
    char *final_strip = (char *)malloc(256);
    memcpy(final_strip, stripped, 256);
    return final_strip;
}

char **tokenize(char *command, const char delim[2])
{
    char **args = (char **)malloc(256 * sizeof(char *));
    int count = 0;
    char *token = strtok(command, delim);
    while (token != NULL)
    {
        args[count++] = strip(token);
        token = strtok(NULL, delim);
    }
    return args;
}

int pipe_process(char *command)
{
    char **cmds = tokenize(command, "|");
    int fd[2];
    if (pipe(fd) == -1)
    {
        perror("error in piping");
    }
    int pid1 = fork();
    if (pid1 < 0)
    {
        perror("error");
    }
    else if (pid1 == 0)
    {
        char **args = tokenize(cmds[0], " ");
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);
        int check = execvp(args[0], args);
        if (check == -1)
        {
            printf("Error running execvp system call\n");
            return -1;
        }
    }
    int pid2 = fork();
    if (pid2 < 0)
    {
        perror("error");
    }
    else if (pid2 == 0)
    {
        char **args = tokenize(cmds[1], " ");
        close(fd[1]);
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);
        int check = execvp(args[0], args);
        if (check == -1)
        {
            printf("Error running execvp system call\n");
            return -1;
        }
    }
    close(fd[0]);
    close(fd[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    return 0;
}

void shell_loop()
{
    int status;
    do
    {
        char *user = getenv("USER");
        char host[256];
        int hostname = gethostname(host, sizeof(host));
        printf("%s@%s~$ ", user, host);

        char *command = read_user_input();
        command = strtok(command, "\n");

        if (strchr(command, '|'))
        {
            // printf("pipe exists\n");
            status = pipe_process(command);
        }else{
        char **args = tokenize(command, " ");
        // for (int i = 0; args[i] != NULL; i++)
        // {
        //     printf("%s\n", args[i]);
        // }
        status = launch(args);
        }
    } while (status);
}

int main()
{
    shell_loop();
    return 0;
}