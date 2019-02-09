#include "my_shell.h"

int exportVars(char **command, int numOfWords, list *vars) {
    int i;
    for (i = 1; i < numOfWords; i++) {
        char **nameAndValue;
        int numOfParts = parse(command[i], &nameAndValue, '=');
        if (numOfParts == 1) {
            if (getenv(command[i]) != NULL) {
                continue;
            }
            char *value = getVariable(command[i], vars);
            if (value != NULL) {
                setenv(command[i], value, 1);
                continue;
            }
            if (setenv(command[i], "", 1) != 0) {
                perror("while setting env variable");
                return -1;
            }
        } else if (numOfParts == 2) {
            if (setenv(nameAndValue[0], nameAndValue[1], 1) != 0) {
                perror("while setting env variable");
                return -1;
            }
        } else {
            int t;
            int sizeOfValue = numOfParts;
            for (t = 1; t < numOfParts; t++) {
                sizeOfValue += strlen(command[t]);
            }
            char *value = malloc(sizeof(char) * sizeOfValue);
            int p = 0;
            for (t = 1; t < numOfParts; t++) {
                int k;
                for (k = 0; k < (int) strlen(command[t]); k++) {
                    value[p] = command[t][k];
                    p++;
                }
                if (t != numOfParts - 1) {
                    value[p] = '=';
                }
                p++;
            }
            value[p] = '\0';
            if (setenv(nameAndValue[0], value, 1) != 0) {
                perror("while setting env variable");
                return -1;
            }
            free(value);
        }
        freeParsed(&nameAndValue, numOfParts);
    }
    return 0;
}

int variableDefinition(char **command, int numOfWords, list *vars) {
    char **nameOfVars = malloc(sizeof(char *) * numOfWords);
    char **valueOfVars = malloc(sizeof(char *) * numOfWords);
    if (!valueOfVars || !nameOfVars) {
        perror("while creating array of variables");
        exit(-1);
    }
    int numOfDefined = 0;
    int i;
    for (i = 0; i < numOfWords; i++) {
        if (i != numOfDefined || command[i][0] == '=') {
            break;
        }
        int t;
        int numOfLetters = (int) strlen(command[i]);
        for (t = 1; t < numOfLetters; t++) {
            if (command[i][t] == '=') {
                nameOfVars[numOfDefined] = malloc(sizeof(char) * (t + 1));
                valueOfVars[numOfDefined] = malloc(sizeof(char) * (numOfLetters - t));
                if (!nameOfVars[numOfDefined] || !valueOfVars[numOfDefined]) {
                    perror("while creating array of variables");
                    exit(-1);
                }
                int p;
                for (p = 0; p < t; p++) {
                    nameOfVars[numOfDefined][p] = command[i][p];
                }
                nameOfVars[numOfDefined][p] = '\0';
                int k = 0;
                for (p = p + 1; p < numOfLetters; p++) {
                    valueOfVars[numOfDefined][k] = command[i][p];
                    k++;
                }
                valueOfVars[numOfDefined][k] = '\0';
                numOfDefined++;
            }
        }
    }
    if (numOfDefined == numOfWords) {
        for (i = 0; i < numOfDefined; i++) {
            setVariable(nameOfVars[i], valueOfVars[i], vars);
        }
    } else if (strcmp(command[numOfDefined], "export") == 0) {
        for (i = numOfDefined + 1; i < numOfWords; i++) {
            int k;
            for (k = 0; k < numOfDefined; k++) {
                if (strcmp(command[i], nameOfVars[k]) == 0) {
                    if (setenv(nameOfVars[k], valueOfVars[k], 1) != 0) {
                        perror("while setting env variable");
                        return -1;
                    }
                }
            }
        }
    }
    freeParsed(&valueOfVars, numOfDefined);
    freeParsed(&nameOfVars, numOfDefined);
    return numOfDefined;
}


int setVariable(char *varName, char *varValue, list *vars) {
    int i;
    for (i = 0; i < vars->next; i++) {
        if (strcmp(vars->names[i], varName) == 0) {
            vars->values[i] = realloc(vars->values[i], (strlen(varValue) + 1) * sizeof(char));
            int t;
            for (t = 0; t < (int) strlen(varValue); t++) {
                vars->values[i][t] = varValue[t];
            }
            vars->values[i][t] = '\0';
            if (getenv(varName) != NULL) {
                setenv(varName, varValue, 1);
            }
            return 0;
        }
    }
    if (vars->next == vars->size) {
        int newSize = vars->size + vars->initialSize;
        vars->names = realloc(vars->names, newSize);
        vars->values = realloc(vars->values, newSize);
        if (!vars->values || !vars->names) {
            perror("while increasing array of variables");
            exit(-1);
        }
        vars->size = newSize;
    }
    vars->values[vars->next] = malloc(sizeof(char) * (strlen(varValue) + 1));
    vars->names[vars->next] = malloc(sizeof(char) * (strlen(varName) + 1));
    if (!vars->values[vars->next] || !vars->names[vars->next]) {
        perror("while expanding array of variables");
        exit(-1);
    }
    for (i = 0; i < (int) strlen(varValue); i++) {
        vars->values[vars->next][i] = varValue[i];
    }
    vars->values[vars->next][i] = '\0';
    for (i = 0; i < (int) strlen(varName); i++) {
        vars->names[vars->next][i] = varName[i];
    }
    vars->names[vars->next][i] = '\0';
    vars->next++;
    return 0;
}

char *getVariable(char *varName, list *vars) {
    char *value = getenv(varName);
    if (value != NULL) {
        return value;
    }
    int i;
    for (i = 0; i < vars->next; i++) {
        if (strcmp(vars->names[i], varName) == 0) {
            return vars->values[i];
        }
    }
    return (char *) NULL;
}

int substituteVariables(char ***words, int numOfWords, list *vars) {
    char **changed = *words;
    int i;
    for (i = 0; i < numOfWords; i++) {
        int p;
        for (p = 0; p < (int) strlen(changed[i]); p++) {
            if (changed[i][p] == '$') {
                char *varName = changed[i] + p + 1;
                char *varValue = getVariable(varName, vars);
                if (varValue == NULL) {
                    varValue = "";
                }
                int newLength= (int) strlen(varValue) + p + 1;
                changed[i] = realloc(changed[i], sizeof(char) * newLength);
                int t;
                int k=0;
                for (t = p; t < newLength-1; t++) {
                    changed[i][t] = varValue[k];
                    k++;
                }
                changed[i][t] = '\0';
                continue;
            }
        }
    }
    return 0;
}
