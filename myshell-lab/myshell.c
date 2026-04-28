#include "myshell.h"


int main(int argc, char **argv) {
    char line[MAX_BUFFER];
    char *args[MAX_ARGS];
    char shell_path[PATH_MAX];

    
    realpath(argv[0], shell_path);
    setenv("shell", shell_path, 1);

    FILE *input = stdin;
    if (argc > 1) { 
        input = fopen(argv[1], "r");
        if (!input) {
            perror("myshell: batch file");
            return 1;
        }
    }

    while (1) {
        if (input == stdin) printf("myshell> ");

        if (!fgets(line, MAX_BUFFER, input)) break;

        
        int i = 0;
        args[i] = strtok(line, SEPARATORS);
        while (args[i] != NULL && i < MAX_ARGS - 1) {
            args[++i] = strtok(NULL, SEPARATORS);
        }

        if (args[0] == NULL) continue;

        
        if (strcmp(args[0], "quit") == 0) break;
        else if (strcmp(args[0], "cd") == 0) internal_cd(args);
        else if (strcmp(args[0], "clr") == 0) system("clear");
        else if (strcmp(args[0], "dir") == 0) internal_dir(args);
        else if (strcmp(args[0], "environ") == 0) internal_environ();
        else if (strcmp(args[0], "echo") == 0) internal_echo(args);
        else if (strcmp(args[0], "help") == 0) internal_help();
        else if (strcmp(args[0], "pause") == 0) internal_pause();
        else {
            
            pid_t pid = fork();
            if (pid == 0) { 
                setenv("parent", shell_path, 1);
                if (execvp(args[0], args) == -1) {
                    perror("myshell");
                }
                exit(EXIT_FAILURE);
            } else if (pid < 0) {
                perror("myshell: fork");
            } else {
                waitpid(pid, NULL, 0); 
            }
        }
    }

    if (input != stdin) fclose(input);
    return 0;
}