#ifndef LAB4_MY_SHELL_H
#define LAB4_MY_SHELL_H

#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/param.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUF 1024
#define MAXHOSTNAMELEN 256

typedef struct {
    char **names;
    char **values;
    int size;
    int next;
    int initialSize;
} list;

int invitation();

int readInput(char **input);

int parse(char *input, char ***parsed, char delim);

int execute(char **commands, int numOfCommands, list *vars);

int execBuiltIn(char **command, int numOfWords, list *vars);

int exportVars(char **command, int numOfWords, list *vars);

int setVariable(char *varName, char *varValue, list *vars);

int variableDefinition(char **command, int numOfWords, list *vars);

char *getVariable(char *varName, list *vars);

int substituteVariables(char ***words, int numOfWords, list *vars);

int isBuiltIn(char *command);

int freeParsed(char ***arrayOfStrings, int numOfElements);

#endif //LAB4_MY_SHELL_H
