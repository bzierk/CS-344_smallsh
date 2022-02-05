#include "smallsh.h"

// A struct to hold commands, args, special symbols, and file names parsed from a user's input
struct new_command {
    int argc;
    char **argv;
    char *in_path;
    char *out_path;
    int in_redir;
    int out_redir;
    int bg_proc;
    int comment;
};

/*
 * Initialize a new_command struct so valgind stops telling me I'm using uninitialized values.
 */
void initialize_cmd(struct new_command *cmd) {
    cmd->argc = 0;
    cmd->argv = NULL;
    cmd->in_path = NULL;
    cmd->out_path = NULL;
    cmd->in_redir = 0;
    cmd->out_redir = 0;
    cmd->bg_proc = 0;
    cmd->comment = 0;
}

void clean_up_cmd(struct new_command **cmd) {
    struct new_command* currCmd = *cmd;
    for (int i = 0; i < currCmd->argc; i++) {
        if (currCmd->argv[i] != NULL) {
            free(currCmd->argv[i]);
        }
    }
    if (currCmd->in_path != NULL) {
        free(currCmd->in_path);
    }
    if (currCmd->out_path != NULL) {
        free(currCmd->out_path);
    }

//    free(currCmd);
}

/*
 * Used for debugging, prints the contents of a new_command struct
 */
void print_cmd_struct(struct new_command *cmd) {
    printf("argc: %d\n", cmd->argc);
    printf("argv: ");
    for (int i = 0; i < cmd->argc; i++) {
        printf("%s, ", cmd->argv[i]);
    }
    printf("\n");
    printf("in_path: %s\n", cmd->in_path);
    printf("out_path: %s\n", cmd->out_path);
    printf("redirect in: %d\n", cmd->in_redir);
    printf("redirect out: %d\n", cmd->out_redir);
    printf("bg process: %d\n", cmd->bg_proc);
}

void expand_dollar_signs(char *word) {
    int pid_int;
    char pid_str[8] = {0}; // Default max pid for 64 bit is 4194304, so we must hold 7 + 1 characters to hold null term
    char *temp_str[2048]; // Temp array to hold max length arg
    char *temp_ptr;
    int i = 0;

    pid_int = getpid();
    sprintf(pid_str, "%d", pid_int);

    if (strstr(word, "$$") != NULL) {
        while (strstr(word, "$$") != NULL) {
            temp_ptr = strstr(word, "$$");

            memcpy(temp_str[i], )

        }

        strcpy(word, temp_str);
    }
}

/**
 * Takes a pointer to a line as its parameter. The line is tokenized using " " as a delimiter and an array of tokens
 * is created. The array of tokens is then parsed and split into a new_command struct
 * @param line
 * @return a populated new_command struct
 */
struct new_command *parse_line(char *line) {
    struct new_command *cmd = malloc(sizeof(struct new_command));
    int i = 0, j = 0, n = 0;
    char *words[MAX_ARGS] = {NULL};
    char **tmpPtr = {NULL};

    initialize_cmd(cmd);

    // Initiate an array which can contain a maximum of 512 args, using strtok with a delimiter of " ", parse the input
    // line and store each token in the array.
    char *strip_newline_token = strtok(line, "\n");
    char *token = strtok(strip_newline_token, " ");
    while (token != NULL) {
        expand_dollar_signs(token);
        words[i] = calloc(strlen(token) + 1, sizeof(char));
        strcpy(words[i], token);
        i++;
        token = strtok(NULL, " ");
    }

    // Initialize argv to hold "i" number of pointers where i is the number of tokens in the user's input and could
    // potentially be the maximum number of argv. Each of these pointers will be used to hold an individual
    // argument in the argv array
    cmd->argv = malloc(i * sizeof(char *));

    // Iterate through the array and generate the command struct. Words can be stored in the args array, if the special
    // characters "<", ">", or "&" are present, the corresponding flag in the struct will be set to "1".
    while (words[j] != NULL) {

        // Check if the current token is the input redirect symbol. If it matches, set the input redirect flag and
        // store the next token as the path for input redirection
        if (strcmp(words[j], "<") == 0) {
            cmd->in_redir = 1;
            cmd->in_path = calloc(strlen(words[j + 1]) + 1, sizeof(char));
            strcpy(cmd->in_path, words[j + 1]);
            j++;
        }

        // Check if the current token is the output redirect symbol. If it matches, set the output redirect flag and
        // store the next token as the path for output redirection
        else if (strcmp(words[j], ">") == 0) {
            cmd->out_redir = 1;
            cmd->out_path = calloc(strlen(words[j + 1]) + 1, sizeof(char));
            strcpy(cmd->out_path, words[j + 1]);
            j++;
        }

        // Check if the current token is the background process symbol. If it matches and the next token is "NULL", this
        // is the last argument, set the background process flag
        else if (strcmp(words[j], "&") == 0) {
            if (words[j + 1] == NULL) {
                cmd->bg_proc = 1;
            }
        }

        // At this point, the token is not an input/output/bg flag, so it must be an argument, allocate space for the
        // token and append it to argv.
        else {
            cmd->argv[n] = calloc(strlen(words[j]) + 1, sizeof(char));
            strcpy(cmd->argv[n], words[j]);
            n++;
        }

        j++;
    }

    // Store the number of arguments
    cmd->argc = n;

    // If the first token in the array is a hash symbol, set a comment flag so the shell knows to skip the line
    if (cmd->argc > 0) {
        if (strcmp(cmd->argv[0], "#") == 0) {
            cmd->comment = 1;
        }
    }

    // Reallocate argv to remove any unnecessary "NULL" members of the array. Use tmpPtr to hold new array pointer
    // in case realloc fails. On failure, realloc would return NULL and the pointer to cmd->argv would be lost,
    // creating a memory leak.
    tmpPtr = realloc(cmd->argv, cmd->argc * sizeof(char *));
    cmd->argv = tmpPtr;

    for (j = 0; j < i; j++) {
        free(words[j]);
    }

    // Return a pointer to the struct which can be used to execute the command
    return cmd;
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
    if (cmd->argc ==  0) {
        return 1;
    }

    if (cmd->comment == 1) {
        return 2;
    }
    print_cmd_struct(cmd);
    return 1;
}

void run_shell(void) {
    struct new_command *cmd;
    char *currLine = NULL;
    int running = 1;

    // Display command line prompt so user knows the shell is awaiting input
    while(running != 0) {
        printf(": ");
        fflush(stdout);
        currLine = get_line();
        printf("You entered: %s\n", currLine);
        fflush(stdout);
        cmd = parse_line(currLine);

        running = execute_command(cmd);
        // clean up
        free(currLine);
        clean_up_cmd(&cmd);

//        exit(EXIT_SUCCESS);
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
