#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>

#define MAX_LINE 80 /* The maximum length command */

/**
 * After reading user input, the steps are:
 * (1) fork a child process using fork()
 * (2) the child process will invoke execvp()
 * (3) parent will invoke wait() unless command included &
 */
void input_command(char* args[],char* input);
int main(void) {
    int saved_stdin = dup(STDIN_FILENO);
    int saved_stdout = dup(STDOUT_FILENO);
    char *args[MAX_LINE / 2 + 1]; /* command line arguments */
    char *recent_command[MAX_LINE / 2 + 1];
    int should_run = 1; /* flag to determine when to exit program */
    int status;
	char input[MAX_LINE];
    int input_redirect = 0;
    int output_redirect = 0;
    char *input_file = NULL;
    char *output_file = NULL;
	int background = 0;

    while (should_run) {
        printf("my_shell 😀 >");
		input_command(args,input);

        //background running
        if (input[strlen(input) - 1] == '&') {
            background = 1;
            input[strlen(input) - 1] = 0;  // remove "&"
        }


        // 공백을 기준으로 문자열을 분리하여 args 배열에 넣기
        char *token = strtok(input, " ");
        if (strcmp(token, "!!") == 0) {
            if (recent_command[0] == NULL) {
                printf("no commands in history\n");
            } else {
                int i = 0;
                while (i < MAX_LINE / 2 + 1) {
                    args[i] = recent_command[i];
                    i++;
                }
                printf("recent command: ");
                for (int j = 0; j < i; j++) {
                    printf("%s ", args[j]);
                }
                printf("\n");
            }
        } else {
            int i = 0;
            while (token != NULL) {
                if (strcmp(token, "<") == 0) {
                    input_redirect = 1;
                    input_file = strtok(NULL, " ");
                } else if (strcmp(token, ">") == 0) {
                    output_redirect = 1;
                    output_file = strtok(NULL, " ");
                } else {
                    args[i] = token;
                    recent_command[i] = strdup(token);
                    i++;
                }
                token = strtok(NULL, " ");
            }
            args[i] = NULL;  // execvp()는 NULL로 끝나는 인수 배열을 기대합니다.
            recent_command[i] = NULL;
        }

        if (input_redirect) {
            int fd = open(input_file, O_RDONLY);
            if (fd < 0) {
                perror("Input file open error");
                continue;
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        if (output_redirect) {
            int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror("Output file open error");
                continue;
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

		// runs command
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(1);
        } else if (pid == 0) {
            if (execvp(args[0], args) == -1) {
                perror("execvp failed");
            }
            exit(0);  // Ensure child process terminates after execvp fails
        } else {
            if (background) {
                printf("Background process started with PID %d\n", pid);
            } else {
                wait(&status);
            }
        }

        // restores file descriptors.
        if (input_redirect) {
            dup2(saved_stdin, STDIN_FILENO);
        }
        if (output_redirect) {
            dup2(saved_stdout, STDOUT_FILENO);
        }
        fflush(stdout);

    }
    return 0;
}

void input_command(char *args[], char* input) {
   // fgets로 입력받기
        fgets(input, MAX_LINE, stdin);
        // 입력받은 문자열에서 개행 문자 제거
        input[strcspn(input, "\n")] = 0;
        // "&"가 끝에 있으면 백그라운드 실행
}
