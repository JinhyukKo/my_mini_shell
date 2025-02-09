# Mini Shell
> simply runs inner shells using child processes
> * (1) fork a child process using fork()
> * (2) the child process will invoke execvp()
> * (3) parent will invoke wait() unless command included &
> * (4) redirect throughput to files when included ">" "<"
> * (5) Pipe when included "|"

## How to use

Simply it is a shell! 

```bash
> gcc simple-shell.c src/handle_redirection.c src/handle_pipe.c -o a.out
> ./a.out
```

<img width="546" alt="Screenshot 2025-02-08 at 23 16 18" src="https://github.com/user-attachments/assets/ae51d6f2-6d4d-4534-8067-e3e05804ed72" />
