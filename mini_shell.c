#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>

#define MAX_LINE 80 /* The maximum length of a command */

void input_command(char* args[], char* input);

int main(void) {
    int saved_stdin = dup(STDIN_FILENO);
    int saved_stdout = dup(STDOUT_FILENO);
    char *args[MAX_LINE / 2 + 1];  // Command line arguments
    char *recent_command[MAX_LINE / 2 + 1];  // Store recent command
    int should_run = 1;  // Flag to determine when to exit the program
    int status;
    char input[MAX_LINE];  // Store user input
    int input_redirect = 0, output_redirect = 0, pipe_redirect = 0;
    char *input_file = NULL, *output_file = NULL, *following_command = NULL;
    int background = 0;  // Background process flag
    int fd[2];  // Pipe file descriptors

    while (should_run) {
        printf("my_shell 😀 > ");
        input_command(args, input);

        // Handle background process (check for '&' at the end)
        if (input[strlen(input) - 1] == '&') {
            background = 1;
            input[strlen(input) - 1] = 0;  // Remove "&" symbol
        }

        // Tokenize the input command and parse special operators
        char *token = strtok(input, " ");
        if (strcmp(token, "!!") == 0) {
            // Handle history command "!!"
            if (recent_command[0] == NULL) {
                printf("No commands in history\n");
            } else {
                int i = 0;
                while (recent_command[i] != NULL) {
                    args[i] = recent_command[i];
                    i++;
                }
                printf("Recent command: ");
                for (int j = 0; j < i; j++) {
                    printf("%s ", args[j]);
                }
                printf("\n");
            }
        } else {
            int i = 0;
            while (token != NULL) {
                // Parse input redirection
                if (strcmp(token, "<") == 0) {
                    input_redirect = 1;
                    input_file = strtok(NULL, " ");
                }
                // Parse output redirection
                else if (strcmp(token, ">") == 0) {
                    output_redirect = 1;
                    output_file = strtok(NULL, " ");
                }
                // Parse pipe operator
                else if (strcmp(token, "|") == 0) {
                    pipe_redirect = 1;
                    following_command = strtok(NULL, " ");
                }
                // Store command argument
                else {
                    args[i] = token;
                    recent_command[i] = strdup(token);
                    i++;
                }
                token = strtok(NULL, " ");
            }
            args[i] = NULL;  // Null-terminate args array
            recent_command[i] = NULL;  // Null-terminate recent command
        }

        // Handle input redirection
        if (input_redirect) {
            int fd_input = open(input_file, O_RDONLY);
            if (fd_input < 0) {
                perror("Input file open error");
                continue;
            }
            dup2(fd_input, STDIN_FILENO);
            close(fd_input);
        }

        // Handle output redirection
        if (output_redirect) {
            int fd_output = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd_output < 0) {
                perror("Output file open error");
                continue;
            }
            dup2(fd_output, STDOUT_FILENO);
            close(fd_output);
        }

        // Fork and execute the command
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(1);
        } else if (pid == 0) {
            // Handle pipe redirection
            if (pipe_redirect) {
                pipe(fd);
                pid = fork();
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
            // No pipe redirection: execute the command
            else if (execvp(args[0], args) == -1) {
                perror("execvp failed");
            }
        } else {
            // Parent process: wait for the child process to finish
            if (background) {
                printf("Background process started with PID %d\n", pid);
            } else {
                wait(&status);
            }
        }

        // Restore file descriptors if redirected
        if (input_redirect) {
            dup2(saved_stdin, STDIN_FILENO);
        }
        if (output_redirect) {
            dup2(saved_stdout, STDOUT_FILENO);
        }
        fflush(stdout);  // Ensure any buffered output is printed
    }

    return 0;
}

/**
 * Function to get and process user input command
 */
void input_command(char *args[], char* input) {
    // Get user input using fgets
    fgets(input, MAX_LINE, stdin);
    // Remove newline character from the input
    input[strcspn(input, "\n")] = 0;
}

