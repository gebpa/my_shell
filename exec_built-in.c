#include "my_shell.h"

int isBuiltIn(char *command) {
    char builtInS[3][10] = {"exit", "cd", "export"};
    int i;
    for (i = 0; i < 3; i++) {
        if (strcmp(command, builtInS[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

//return 1 if command is NOT built in
//return 0 if command is built in and it was executed
//return -1 if command is built in and an error has occured
//there is no return if there is no pipe and command is exit
int execBuiltIn(char **command, int numOfWords, list *vars) {
    if (strcmp(command[0], "exit") == 0) {
        exit(0);
    } else if (strcmp(command[0], "cd") == 0) {
        if (numOfWords == 1 || strcmp(command[1], "~") == 0) {
            chdir(getenv("HOME"));
            return 0;
        } else {
            if (chdir(command[1]) != 0) {
                perror("while changing working directory");
                return -1;
            }
            return 0;
        }
    } else if (strcmp(command[0], "export") == 0) {
        return exportVars(command, numOfWords, vars);
    }
    return 1;
}
