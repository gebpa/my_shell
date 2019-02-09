#include "my_shell.h"

int main(int args, char *argv[]) {
    write(1, "command interpretator (in idea it should work as ksh)\n",
          strlen("command interpretator (in idea it should work as ksh)\n"));
    list vars = {.initialSize=256, .size=256, .next=0, .names = malloc(256 * sizeof(char *)), .values =malloc(
            256 * sizeof(char *))};
    if (!vars.names || !vars.values) {
        perror("while creating array of variables");
        exit(-1);
    }
    if (args == 1) {
        while (1) {
            invitation();
            char *input;
            readInput(&input);
            char **parsed;
            int numOfPipes = parse(input, &parsed, '|');
            execute(parsed, numOfPipes, &vars);
            freeParsed(&parsed, numOfPipes);
            free(input);
        }
    } else {
        int i;
        for (i = 1; i < args; i++) {
            write(1, argv[i], strlen(argv[i]));
        }
    }
    return 0;
}

int invitation() {
    char hostName[MAXHOSTNAMELEN];
    char cwd[PATH_MAX];
    char *logname = getenv("LOGNAME");
    char *userNameAndHost = (char *) malloc(sizeof(hostName) + sizeof(cwd) + strlen(logname) + 10);
    if (!userNameAndHost) {
        perror("while writing invitation");
        exit(-1);
    }
    gethostname(hostName, MAXHOSTNAMELEN);
    getcwd(cwd, PATH_MAX);
    userNameAndHost[0] = '\0';
    userNameAndHost = strcat(userNameAndHost, logname);
    userNameAndHost = strcat(userNameAndHost, "@");
    userNameAndHost = strcat(userNameAndHost, hostName);
    userNameAndHost = strcat(userNameAndHost, ":");
    userNameAndHost = strcat(userNameAndHost, cwd);
    userNameAndHost = strcat(userNameAndHost, "$ ");
    write(1, userNameAndHost, strlen(userNameAndHost));
    free(userNameAndHost);
    return 0;
}

int readInput(char **changingInput) {
    char *input = (char *) malloc(BUF);
    if (!input) {
        perror("while reading input");
        exit(-1);
    }
    ssize_t size = read(0, input, BUF - 1);
    int mult = 2;
    ssize_t fullLen = size;
    while (input[fullLen - 1] != '\n') {
        int extra = BUF * mult;
        mult++;
        input = realloc(input, extra);
        if (!input) {
            perror("while reading input");
            exit(-1);
        }
        size = read(0, input + fullLen, BUF - 1);
        fullLen += size;
    }
    input[fullLen] = '\0';
    *changingInput = input;
    return 0;
}

int parse(char *input, char ***changingParsed, char delim) {
    int i;
    int numOfDelim = 0;
    for (i = 0; i < (int) strlen(input); i++) {
        if (input[i] == delim) numOfDelim++;
    }
    char **parsed = (char **) malloc((numOfDelim + 2) * sizeof(char *));
    if (!parsed) {
        perror("while parsing");
        exit(-1);
    }
    int numOfParts = 0;
    int lengthOfPart = 0;
    for (i = 0; i < (int) strlen(input) + 1; i++) {
        if (input[i] != delim && input[i] != '\n' && input[i] != '\0') lengthOfPart++;
        else if (lengthOfPart != 0) {
                parsed[numOfParts] = (char *) malloc(lengthOfPart + 1);
                if (!parsed[numOfParts]) {
                    perror("while parsing");
                    exit(-1);
                }
                int t;
                int k = 0;
                for (t = i - lengthOfPart; t < i; t++) {
                    parsed[numOfParts][k] = input[t];
                    k++;
                }
                parsed[numOfParts][k] = '\0';
            lengthOfPart = 0;
            numOfParts++;
        }
    }
    parsed[numOfParts] = NULL;
    *changingParsed = parsed;
    return numOfParts;
}

int execute(char **commands, int numOfCommands, list *vars) {
    int fd[2];
    pid_t p1;
    int in = STDIN_FILENO;
    int i;
    for (i = 0; i < numOfCommands; i++) {
        if (pipe(fd) == -1) {
            perror("while creating pipe");
            exit(-1);
        }
        char **parsedWords;
        int numOfWords = parse(commands[i], &parsedWords, ' ');
        substituteVariables(&parsedWords, numOfWords, vars);
        int numOfSkipedWords = 0;
        if (numOfCommands == 1){
            numOfSkipedWords = variableDefinition(parsedWords, numOfWords, vars);
            if (numOfSkipedWords == numOfWords || numOfSkipedWords == -1) {
                freeParsed(&parsedWords, numOfWords);
                close(fd[1]);
                in = fd[0];
                continue;
            } else if (numOfSkipedWords != 0) {
                parsedWords = parsedWords + numOfSkipedWords;
                numOfWords = numOfWords - numOfSkipedWords;
            }
        }
        int builtIn = isBuiltIn(parsedWords[0]);
        if (builtIn == 0) {
            p1 = fork();
            if (p1 == -1) {
                perror("while forking");
                exit(-1);
            } else if (p1 == 0) {
                //закрвываем файл, связанный с дескриптором fd[0]
                //так child process не читает из данного файла
                close(fd[0]);
                //дескритор 0 будет указавать на файл, связанный с дескритором in
                //стд. вход нового процесса будет файл, соответствующий дескритору in
                dup2(in, STDIN_FILENO);
                if (i != numOfCommands - 1)
                    //дескриптор 1 будет указывать на файл, связанный с дескритором fd[1]
                    //стд. выход нового процесса будет записан в файл, соответствующий дескритору fd[1]
                    dup2(fd[1], STDOUT_FILENO);
                if (execvp(parsedWords[0], parsedWords) < 0) {
                    perror("while calling execvp");
                    exit(-1);
                }
            } else {
                //закрвываем файл, связанный с дескриптором fd[1]
                //так parent process не записывает ничего в данный файл
                close(fd[1]);
                wait(NULL);
                //переменная in равна дескриптору файла, из которого следующий дочерний процесс
                //будет читать
                in = fd[0];
            }
        } else if (builtIn == 1 && numOfCommands != 1) {
            close(fd[1]);
            in = fd[0];
        } else {
            execBuiltIn(parsedWords, numOfWords, vars);
        }
        parsedWords = parsedWords - numOfSkipedWords;
        numOfWords = numOfWords + numOfSkipedWords;
        freeParsed(&parsedWords, numOfWords);
    }
    return 0;
}

int freeParsed(char ***arrayOfStrings, int numOfElements) {
    int i;
    for (i = 0; i < numOfElements; i++) {
        free((*arrayOfStrings)[i]);
    }
    free(*arrayOfStrings);
    return 0;
}

char *convertIntToCharArr(int n) {
    int numOfDigits = 0;
    int sav = n;
    while (sav != 0) {
        sav /= 10;
        numOfDigits++;
    }
    char *arr = calloc(numOfDigits, sizeof(char));
    int i;
    for (i = 0; i < numOfDigits; i++) {
        arr[numOfDigits - 1 - i] = (n % 10) + '0';
        n /= 10;
    }
    return arr;
}
