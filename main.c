#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_INPUT_LENGTH 1024
#define UNKNOWN_COMMAND_ERROR_CODE -1

int return_code;

int echo(int argc, char *argv[]) {
    for (int i = 1; i < argc; ++i) {
        printf("%s%c", argv[i], i == argc - 1 ? '\n' : ' ');
    }
    return argc - 1;
}

int retcode(int argc, char *argv[]) {
    printf("%d\n", return_code);
    return 0;
}

int cmd_executor(char **tokens, int tokens_count) {
    char *command_name = tokens[0];
    if (!strcmp(command_name, "echo")) {
        return echo(tokens_count, tokens);
    } else if (!strcmp(command_name, "retcode")) {
        return retcode(tokens_count, tokens);
    } else {
        return UNKNOWN_COMMAND_ERROR_CODE;
    }
}

int split_by_delimiter(const char *delimiter, char *buf, char ***commands, int *commands_count) {
    char *cmd = strtok(buf, delimiter);
    *commands = NULL;
    int pieces_count = 0;
    while (cmd != NULL) {
        *commands = realloc(*commands, sizeof(char *) * ++pieces_count);
        (*commands)[pieces_count - 1] = cmd;
        cmd = strtok(NULL, delimiter);
    }
    *commands_count = pieces_count;
    return 0;
}

int cmd_parser() {
    char buf[MAX_INPUT_LENGTH];
    while (fgets(buf, MAX_INPUT_LENGTH, stdin) != NULL) {
        char **commands;
        int commands_count;
        split_by_delimiter(";\n", buf, &commands, &commands_count);
        for (int i = 0; i < commands_count; i++) {
            char **tokens;
            int tokens_count;
            split_by_delimiter(" ", commands[i], &tokens, &tokens_count);
            return_code = cmd_executor(tokens, tokens_count);
            free(tokens);
        }
        free(commands);
    }
    return 0;
}

int main(int argc, char *argv[]) {
    cmd_parser();
    return 0;
}
