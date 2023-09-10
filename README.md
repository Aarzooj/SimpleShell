# Simple Shell
- A Simple Shell is implemented in C that waits for user input, executes commands provided in the userinput, and then repeats until terminated using ctrl+c.
- System calls like fork() and execvp() are used to implement this.
- Pipes are implemented through a separate functions.
- Different handler functions are implemented to handle SIGINT and SIGCHLD signals.