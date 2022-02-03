#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CHAR_LEN 2048
#define MAX_ARGS 512

// A struct to hold commands, args, special symbols, and file names parsed from a user's input
struct new_command {
    char *cmd;
    char *stdin;
    char *stdout;
    int in_redir;
    int out_redir;
    int bg_proc;
};

/**
 * Takes a pointer to a line as its parameter.
 * @param line
 * @return a populated new_command struct
 */
struct new_command *parse_line(char *line) {
    int i = 0;
    char *words[MAX_ARGS];

    // Initiate an array which can contain a maximum of 512 args, using strtok with a delimiter of " ", parse the input
    // line and store each token in the array.
    char *token = strtok(line, " ");
    while (token != NULL) {
        words[i] = calloc(strlen(token) + 1, sizeof(char));
        strcpy(words[i], token);
        i++;
        token = strtok(NULL, " ");
    }

    // Iterate through the array and generate the command struct. Words can be stored in the args array, if the special
    // characters "<", ">", or "&" are present, the corresponding flag in the struct will be set to "1".
    for (int j = 0; j < MAX_ARGS; j++) {

    }

    // Return a pointer to the struct which can be used to execute the command

}

/**
 * Get a line from stdin. getline returns the number of characters read or -1 on error so if line_size is -1,
 * the line should not be returned, and it will exit with a failure status. If getline is successful, the function
 * returns a pointer to the line.
 */
char *get_line(void) {
    char *currLine = NULL;
    size_t curr_line_size = 0;
    ssize_t line_size;

    line_size = getline(&currLine, &curr_line_size, stdin);
    if (line_size == -1) {
        exit(EXIT_FAILURE);
    }

    return currLine;
}

int execute_command(struct new_command *cmd) {

}

void run_shell(void) {
    char *currLine = NULL;
    int running = 1;

    // Display command line prompt so user knows the shell is awaiting input
    while(running != 0) {
        printf(": ");
        fflush(stdout);
        currLine = get_line();
        printf("You entered: %s\n", currLine);
        fflush(stdout);
        struct new_command *cmd = parse_line(currLine);

        running = execute_command(cmd);
    }

    exit(EXIT_SUCCESS);
}

/**
 * Serves as the entry point to the program, calls the run_shell driver function and will exit with a success status
 * when the user terminates run_shell.
 */
int main() {
    // Loop run shell command
    run_shell();

    // Any necessary clean up and exit
    exit(EXIT_SUCCESS);
}
