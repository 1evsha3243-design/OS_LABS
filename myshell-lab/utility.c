#include "myshell.h"

extern char **environ;

void internal_cd(char **args) {
    if (args[1] == NULL) {
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        }
    } else {
        if (chdir(args[1]) != 0) {
            perror("myshell: cd");
        } else {
            char cwd[PATH_MAX];
            getcwd(cwd, sizeof(cwd));
            setenv("PWD", cwd, 1);
        }
    }
}

void internal_dir(char **args) {
    char *path = (args[1] == NULL) ? "." : args[1];
    DIR *dir = opendir(path);
    if (dir == NULL) {
        perror("myshell: dir");
        return;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        printf("%s  ", entry->d_name);
    }
    printf("\n");
    closedir(dir);
}

void internal_environ() {
    for (char **env = environ; *env != 0; env++) {
        printf("%s\n", *env);
    }
}

void internal_echo(char **args) {
    for (int i = 1; args[i] != NULL; i++) {
        printf("%s%s", args[i], (args[i+1] != NULL) ? " " : "");
    }
    printf("\n");
}

void internal_help() {
    printf("\n--- Руководство пользователя myshell ---\n");
    printf("cd <directory>    : Смена текущего каталога. Без аргументов выводит текущий путь.\n");
    printf("clr               : Очистка экрана терминала.\n");
    printf("dir <directory>   : Вывод содержимого указанного каталога.\n");
    printf("environ           : Вывод всех переменных окружения.\n");
    printf("echo <comment>    : Вывод сообщения на экран.\n");
    printf("help              : Вывод этого справочного руководства.\n");
    printf("pause             : Приостановка работы оболочки до нажатия клавиши Enter.\n");
    printf("quit              : Завершение работы и выход из оболочки.\n");
    printf("----------------------------------------\n");
}
void internal_pause() {
    printf("Press Enter to continue...");
    while (getchar() != '\n');
}