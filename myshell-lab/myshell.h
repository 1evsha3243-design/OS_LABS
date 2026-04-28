#ifndef MYSHELL_H
#define MYSHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/wait.h>
#include <limits.h>

#define MAX_BUFFER 1024
#define MAX_ARGS 64
#define SEPARATORS " \t\n"

// Прототипы функций
void execute_command(char **args);
void internal_cd(char **args);
void internal_dir(char **args);
void internal_environ();
void internal_echo(char **args);
void internal_help();
void internal_pause();

#endif