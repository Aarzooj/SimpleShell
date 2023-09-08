#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>

#define INPUT_SiZE 256
#define HISTORY_SIZE 100

struct CommandParameter
{
    char command[INPUT_SiZE];
    time_t start_time;
    time_t end_time;
    double duration;
    pid_t process_pid;
};

struct CommandHistory
{
    struct CommandParameter record[HISTORY_SIZE];
    int historyCount;
};

struct CommandHistory history;

void displayTerminate()
{

    for (int i = 0; i < history.historyCount; i++)
    {
        struct CommandParameter record = history.record[i];
        printf("%s %d\n", record.command, record.process_pid);
        printf("%s %s %.2lf\n", ctime(&record.start_time), ctime(&record.end_time), record.duration);
        printf("--------------------------------\n");
    }
}

static void my_handler(int signum) {
    displayTerminate();
    exit(1);
}

void displayHistory()
{
    history.record[history.historyCount].process_pid = getpid();
    for (int i = 0; i < history.historyCount + 1; i++)
    {
        printf("%d  %s\n", i + 1, history.record[i].command);
    }
}

int create_process_and_run(char **args)
{
    int status = fork();
    if (status < 0)
    {
        printf("Error");
    }
    else if (status == 0)
    {
        history.record[history.historyCount].process_pid = getpid();
        int check = execvp(args[0], args);
        if (check == -1)
        {
            printf("Error running execvp system call\n");
            return -1;
        }
    }
    else
    {
        int child_status;
        wait(&child_status); // Wait for the child to complete
        if (WIFEXITED(child_status))
        {
            int exit_code = WEXITSTATUS(child_status);
            // printf("Child process exited with status: %d\n", exit_code);
        }
        else
        {
            printf("Child process did not exit normally.\n");
        }
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

int pipe_process(char **cmds, int pipes)
{
    int fd[pipes][2];
    for (int i = 0; i < pipes; i++)
    {
        if (pipe(fd[i]) == -1)
        {
            perror("Piping failed\n");
        }
    }
    int pid;
    for (int i = 0; i < pipes + 1; i++)
    {
        char **args = tokenize(cmds[i], " ");
        pid = fork();
        history.record[history.historyCount].process_pid = getpid();
        if (pid < 0)
        {
            perror("error\n");
        }
        else if (pid == 0)
        {
            if (i > 0)
            {
                for (int j = 0; j < pipes; j++)
                {
                    if (j != i)
                    {
                        close(fd[j][1]);
                    }
                    if (j != i - 1)
                    {
                        close(fd[j][0]);
                    }
                }
                dup2(fd[i - 1][0], STDIN_FILENO);
                close(fd[i - 1][0]);
            }
            if (i < pipes)
            {
                dup2(fd[i][1], STDOUT_FILENO);
                close(fd[i][1]);
            }
            int check = execvp(args[0], args);
            if (check == -1)
            {
                printf("Error running execvp system call\n");
                return -1;
            }
        }
        else
        {
            if (i > 0)
            {
                close(fd[i - 1][0]);
            }
            if (i < pipes)
            {
                close(fd[i][1]);
            }
        }
    }

    for (int i = 0; i < pipes + 1; i++)
    {
        wait(NULL);
    }
    return pid;
}

int launch_pipe(char *command)
{
    int status;
    int count = 0;
    for (int i = 0; command[i] != '\0'; i++)
    {
        if (command[i] == '|')
        {
            count++;
        }
    }
    char **cmds = tokenize(command, "|");
    status = pipe_process(cmds, count);
    history.record[history.historyCount].end_time = time(NULL);

    history.record[history.historyCount].duration = difftime(
        history.record[history.historyCount].end_time,
        history.record[history.historyCount].start_time);
    return status;
}

void shell_loop()
{   
    if (signal(SIGINT, my_handler) == SIG_ERR) {
        perror("Signal handling failed");
    }
    
    int status;
    do
    {
        char *user = getenv("USER");
        char host[256];
        int hostname = gethostname(host, sizeof(host));
        printf("%s@%s~$ ", user, host);

        char *command = read_user_input();
        command = strtok(command, "\n");
        char *tmp = strdup(command);
        if (tmp == NULL)
        {
            perror("Error in strdup");
            exit(EXIT_FAILURE);
        }
        if (strstr(command, "history"))
        {
            if (history.historyCount > 0)
            {
                strcpy(history.record[history.historyCount].command, tmp);
                history.record[history.historyCount].start_time = time(NULL);
                displayHistory();
                history.record[history.historyCount].end_time = time(NULL);

                history.record[history.historyCount].duration = difftime(
                    history.record[history.historyCount].end_time,
                    history.record[history.historyCount].start_time);
                history.historyCount++;
            }
            else
            {
                printf("No command in the history\n");
            }
        }
        else
        {
            if (strchr(command, '|'))
            {
                strcpy(history.record[history.historyCount].command, tmp);
                history.record[history.historyCount].start_time = time(NULL);
                status = launch_pipe(command);
                history.historyCount++;
            }
            else
            {
                char **args = tokenize(command, " ");
                strcpy(history.record[history.historyCount].command, tmp);
                history.record[history.historyCount].start_time = time(NULL);

                status = launch(args);

                history.record[history.historyCount].end_time = time(NULL);

                history.record[history.historyCount].duration = difftime(
                    history.record[history.historyCount].end_time,
                    history.record[history.historyCount].start_time);
                history.historyCount++;
            }
        }
    } while (status);
}

int main()
{
    history.historyCount = 0;
    shell_loop();
    return 0;
}