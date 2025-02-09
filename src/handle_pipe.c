#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "../header/handle_pipe.h"

void handle_pipe(char* args[], char *following_command,int saved_stdout) {
    int fd[2];
    pipe(fd);
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
    } else if (pid == 0) {
        // First child process: execute the first command and write to the pipe
        close(fd[0]);  // Close reading end of the pipe
        dup2(fd[1], STDOUT_FILENO);  // Redirect stdout to pipe
        close(fd[1]);
        if (execvp(args[0], args) == -1) {
            perror("execvp failed");
        }
    } else {
        // Parent process: execute the second command and read from the pipe
        wait(NULL);
        close(fd[1]);  // Close writing end of the pipe
        dup2(fd[0], STDIN_FILENO);  // Redirect stdin to pipe
        close(fd[0]);
        dup2(saved_stdout, STDOUT_FILENO);  // Restore stdout
        char *args2[] = {following_command, NULL};
        if (execvp(following_command, args2) == -1) {
            perror("execvp failed");
        }
    }
}
