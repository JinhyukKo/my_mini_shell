#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "../header/handle_redirection.h"

void handle_input_redirection(const char *input_file) {
    int fd_input = open(input_file, O_RDONLY);
    if (fd_input < 0) {
        perror("Input file open error");
        exit(1);
    }
    dup2(fd_input, STDIN_FILENO);  // 표준 입력을 파일로 리디렉션
    close(fd_input);
}

void handle_output_redirection(const char *output_file) {
    int fd_output = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_output < 0) {
        perror("Output file open error");
        exit(1);
    }
    dup2(fd_output, STDOUT_FILENO);  // 표준 출력을 파일로 리디렉션
    close(fd_output);
}
