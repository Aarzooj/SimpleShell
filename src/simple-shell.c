#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <stdbool.h>

#define INPUT_SIZE 256
#define HISTORY_SIZE 100
#define MAX_BGPROCESS 5

pid_t running_bg_process[MAX_BGPROCESS] = {0};

int bgProcess = 0;

struct CommandParameter
{
    char command[INPUT_SIZE];
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
    printf("--------------------------------\n");
    for (int i = 0; i < history.historyCount; i++)
    {
        struct CommandParameter record = history.record[i];
        struct tm *start_time_info = localtime(&record.start_time);
        char start_time_buffer[80];
        strftime(start_time_buffer, sizeof(start_time_buffer), "%Y-%m-%d %H:%M:%S", start_time_info);
        struct tm *end_time_info = localtime(&record.end_time);
        char end_time_buffer[80];
        strftime(end_time_buffer, sizeof(end_time_buffer), "%Y-%m-%d %H:%M:%S", end_time_info);
        printf("%s\nProcess PID: %d\n", record.command, record.process_pid);
        printf("Start time: %s\nEnd Time: %s\nProcess Duration: %f\n", start_time_buffer, end_time_buffer, record.duration);
        printf("--------------------------------\n");
    }
}

static void my_handler(int signum)
{
    printf("\n");
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

int append(pid_t pid)
{
    int added = -1;
    if (running_bg_process[0] == 0 && running_bg_process[1] == 0 && running_bg_process[2] == 0 && running_bg_process[3] == 0 && running_bg_process[4] == 0)
    {
        added = 0;
        running_bg_process[0] = pid;
        return added;
    }
    for (int i = MAX_BGPROCESS - 2; i >= 0; i--)
    {
        if (running_bg_process[i] != 0)
        {
            running_bg_process[i + 1] = pid;
            added = i + 1;
        }
    }
    return added;
}

pid_t pop(pid_t pid)
{
    for (int i = 0; i < MAX_BGPROCESS; i++)
    {
        if (running_bg_process[i] == pid)
        {
            running_bg_process[i] = 0;
            return i;
        }
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

        int check = execvp(args[0], args);
        if (check == -1)
        {
            printf("Error running execvp system call\n");
            return -1;
        }
    }
    else
    {
        if (!(bgProcess))
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
        else
        {
            int order = append(status);
            if (order != -1)
            {
                history.record[history.historyCount].process_pid = status;
                printf("[%d] %d\n", order + 1, status);
            }
            else
            {
                printf("No more background processes can be added");
            }
        }
    }
    return status;
}

int launch(char **args)
{
    int status;
    status = create_process_and_run(args);
    if (status > 0)
    {
        history.record[history.historyCount].process_pid = status;
    }
    else
    {
        history.record[history.historyCount].process_pid = 0;
    }
    return status;
}

char *read_user_input()
{
    char *input = (char *)malloc(INPUT_SIZE);
    if (input == NULL)
    {
        perror("Error in malloc");
        free(input);
        exit(EXIT_FAILURE);
    }
    size_t size = 0;
    int read = getline(&input, &size, stdin);
    if (read != -1)
    {
        return input;
    }
    else
    {
        perror("Error while reading line\n");
        free(input);
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
    char *final_strip = (char *)malloc(INPUT_SIZE);
    memcpy(final_strip, stripped, INPUT_SIZE);
    return final_strip;
}

char **tokenize(char *command, const char delim[2])
{
    char **args = (char **)malloc(INPUT_SIZE * sizeof(char *));
    int count = 0;
    char *token = strtok(command, delim);
    while (token != NULL)
    {
        args[count++] = strip(token);
        token = strtok(NULL, delim);
    }
    if (count > 0 && strcmp(args[count - 1], "&") == 0 && strcmp(delim, " ") == 0)
    {
        bgProcess = 1;
        free(args[count - 1]); // Remove the "&" from the args list
        args[count - 1] = NULL;
        count--;
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
    if (status > 0)
    {
        history.record[history.historyCount].process_pid = status;
    }
    else
    {
        history.record[history.historyCount].process_pid = 0;
    }
    history.record[history.historyCount].end_time = time(NULL);

    history.record[history.historyCount].duration = difftime(
        history.record[history.historyCount].end_time,
        history.record[history.historyCount].start_time);
    free(cmds);
    return status;
}

void handle_sigchld(int signum)
{
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        // printf("Background process with PID %d terminated\n", pid);
        for (int i = 0; i < history.historyCount; i++)
        {
            if (history.record[i].process_pid == pid)
            {
                history.record[i].end_time = time(NULL);

                history.record[i].duration = difftime(
                    history.record[i].end_time,
                    history.record[i].start_time);
                int order = pop(pid);
                char *tmp = history.record[i].command;
                tmp = strtok(tmp, "&");
                printf("\n[%d]+ Done                    %s\n", order + 1, tmp);
                break;
            }
        }
    }
}

bool validate_command(char *command)
{
    if (strchr(command, '\\') || strchr(command, '\"') || strchr(command, '\''))
    {
        return true;
    }
    return false;
}

void shell_loop()
{
    if (signal(SIGINT, my_handler) == SIG_ERR)
    {
        perror("SIGINT handling failed");
    }
    if (signal(SIGCHLD, handle_sigchld) == SIG_ERR)
    {
        perror("SIGCHLD handling failed");
    }
    int status;
    do
    {
        char *user = getenv("USER");
        if (user == NULL)
        {
            perror("USER environment variable not declared");
            exit(1);
        }
        char host[INPUT_SIZE];
        int hostname = gethostname(host, sizeof(host));
        if (hostname == -1)
        {
            perror("gethostname");
            exit(1);
        }
        printf("%s@%s~$ ", user, host);

        char *command = read_user_input();
        if (strlen(command) == 0 || strcmp(command, "\n") == 0)
        {
            status = 1;
            continue;
        }
        command = strtok(command, "\n");
        bool isInvalidCommand = validate_command(command);
        char *tmp = strdup(command);
        if (tmp == NULL)
        {
            perror("Error in strdup");
            exit(EXIT_FAILURE);
        }
        if (isInvalidCommand)
        {
            status = 1;
            strcpy(history.record[history.historyCount].command, tmp);
            history.record[history.historyCount].start_time = time(NULL);
            history.record[history.historyCount].end_time = time(NULL);
            history.record[history.historyCount].duration = difftime(
                history.record[history.historyCount].end_time,
                history.record[history.historyCount].start_time);
            history.historyCount++;
            printf("Invalid Command : includes quotes/backslach\n");
            continue;
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
                status = 1;
                printf("No command in the history\n");
                continue;
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
        bgProcess = 0;
    } while (status);
}

int main()
{
    history.historyCount = 0;
    shell_loop();
    return 0;
}