#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/wait.h>

int create_process_and_run(char *command) {
    // In a real implementation, you would use fork/exec or system() to run the command.
    // Here, we'll just print the command and return a dummy status.
    printf("Running command: %s\n", command);

    // trying fork
    int status = fork();
    if (status<0){
        printf("error");
    }else if (status == 0){
        printf("child\n");
    }else{
        printf("parent\n");
    }
    return 0;
}

int launch (char *command) {
 int status;
 status = create_process_and_run(command);
 return status;
}

char* read_user_input() {
    char* input = NULL;
    size_t bufsize = 0;
    getline(&input, &bufsize, stdin);
    return input;
}

void shell_loop() {
    int status;
    do {
        printf("iiitd@possum:~$ ");
        char* command = read_user_input();
        status = launch(command);
    } while(status);
}

int main(){
    shell_loop();
    return 0;
}