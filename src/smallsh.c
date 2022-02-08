#include "smallsh.h"

int proc_status = 0;

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
    for (int i = 0; i < (currCmd->argc + 1); i++) {
//        if (currCmd->argv[i] != NULL) {
            free(currCmd->argv[i]);
//        }
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
    for (int i = 0; i < (cmd->argc + 1); i++) {
        printf("%s, ", cmd->argv[i]);
    }
    printf("\n");
    printf("in_path: %s\n", cmd->in_path);
    printf("out_path: %s\n", cmd->out_path);
    printf("redirect in: %d\n", cmd->in_redir);
    printf("redirect out: %d\n", cmd->out_redir);
    printf("bg process: %d\n", cmd->bg_proc);
}

void expand_dollar_signs(char **word) {
    int pid_int;
    char pid_str[8] = {0}; // Default max pid for 64 bit is 4194304, so we must hold 7 + 1 characters to hold null term
    char temp_str[2048] = {0}; // Temp array to hold max length arg
    char *temp_ptr = &temp_str[0];
    char *word_ptr = *word;

    pid_int = getpid();
    sprintf(pid_str, "%d", pid_int);

    if (strstr(*word, "$$") != NULL) {
        while (*word_ptr != '\0') {
            if (*word_ptr == '$') {
                if (*(word_ptr + 1) != '\0') {
                    if (*(word_ptr + 1) == '$') {
                        memcpy(temp_ptr, pid_str, strlen(pid_str));
                        word_ptr = word_ptr + 2;
                        temp_ptr = temp_ptr + strlen(pid_str);
                    } else {
                        *temp_ptr = *word_ptr;
                        temp_ptr++;
                        word_ptr++;
                    }
                } else {
                    *temp_ptr = *word_ptr;
                    temp_ptr++;
                    word_ptr++;
                }
            } else {
                *temp_ptr = *word_ptr;
                word_ptr++;
                temp_ptr++;
            }
        }
        *word = malloc(strlen(temp_str) * sizeof(char) + 1);
        strcpy(*word, temp_str);
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
        expand_dollar_signs(&token);
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
        if (strncmp(cmd->argv[0], "#", 1) == 0) {
            cmd->comment = 1;
        }
    }

    // Reallocate argv to remove any unnecessary "NULL" members of the array. Use tmpPtr to hold new array pointer
    // in case realloc fails. On failure, realloc would return NULL and the pointer to cmd->argv would be lost,
    // creating a memory leak.
    tmpPtr = realloc(cmd->argv, (cmd->argc + 1) * sizeof(char *));
    cmd->argv = tmpPtr;

    cmd->argv[cmd->argc] = NULL;

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

int route_output(struct new_command *cmd) {
    int out_fd = 0;
    if (cmd->out_redir == 1) {
        out_fd = open(cmd->out_path, O_WRONLY | O_CREAT | O_TRUNC, 0640);
        if (out_fd == -1) {
            printf("Cannot open %s for output.\n", cmd->out_path);
            fflush(stdout);
            exit(EXIT_FAILURE);
        }
        if (dup2(out_fd, STDOUT_FILENO) == -1) {
            printf("dup2 error: %s\n", strerror(errno));
            fflush(stdout);
            exit(EXIT_FAILURE);
        }
    } else if (cmd->bg_proc == 1) {
        out_fd = open("/dev/null", O_WRONLY);
        if (out_fd == -1) {
            printf("Cannot open /dev/null for input\n");
            fflush(stdout);
            exit(EXIT_FAILURE);
        }
        if (dup2(out_fd, STDOUT_FILENO) == -1) {
            printf("dup2 error: %s\n", strerror(errno));
            fflush(stdout);
            exit(EXIT_FAILURE);
        }
    }
    return out_fd;
}

int route_input(struct new_command *cmd) {
    int in_fd = 0;
    if (cmd->in_redir == 1) {
        in_fd = open(cmd->in_path, O_RDONLY);
        if (in_fd == -1) {
            printf("Cannot open %s for input.\n", cmd->in_path);
            fflush(stdout);
            exit(EXIT_FAILURE);
        }
        if (dup2(in_fd, STDIN_FILENO) == -1) {
            printf("dup2 error: %s", strerror(errno));
            fflush(stdout);
            exit(EXIT_FAILURE);
        }
    } else if (cmd->bg_proc == 1) {
        in_fd = open("/dev/null", O_RDONLY);
        if (in_fd == -1) {
            printf("Cannot open /dev/null for input");
            fflush(stdout);
            exit(EXIT_FAILURE);
        }
        if (dup2(in_fd, STDIN_FILENO) == -1) {
            printf("dup2 error: %s", strerror(errno));
            fflush(stdout);
            exit(EXIT_FAILURE);
        }
    }
    return in_fd;
}

/**
 * Takes a new_command struct as a parameter and executes it in the foreground. Creates a child process by calling
 * fork, the child process then handles routing input/output and attempts to execute the command held at argv[0] by
 * passing it to execvp. Execvp was chosen for ease of finding functions in the PATH  variable (p) and due to the
 * format of variables being held in cmd->argv (v).
 *
 * The parent process will wait for the child process to terminate before returning control to the user.
 * @param cmd
 */
int exec_fg_cmd(struct new_command *cmd) {
    int childStatus;
    pid_t spawnPid = fork();

    switch(spawnPid) {
        case -1:
            perror("Fork failed\n");
            fflush(stdout);
            break;
        case 0:
            // Child case - route input/output, attempt to execute by calling execvp, on failure, display error
            // message, clean up open fd/pid and exit
            route_input(cmd);
            route_output(cmd);
            execvp(cmd->argv[0], cmd->argv);
            perror(cmd->argv[0]);
            _exit(EXIT_FAILURE);
        default:
            // Parent case - Wait for child process to complete, upon completion of child process, store the
            // exit status in proc_status
            spawnPid = waitpid(spawnPid, &childStatus, 0);
            if (WIFEXITED(childStatus) != 0) {
                proc_status = WEXITSTATUS(childStatus);
            } else if (WIFSIGNALED(childStatus) != 0) {
                proc_status = WTERMSIG(childStatus);
            }
            break;
    }
    return 1;
}

/**
 * Takes a new_command struct as a parameter and executes it in the background. Creates a child process by calling
 * fork, the child process then handles routing input/output and attempts to execute the command held at argv[0] by
 * passing it to execvp. If the user does not specify input or output, they will default to dev/null.
 *
 * The parent process will print the PID of the child process and immediately return control to the user.
 * @param cmd
 */
int exec_bg_cmd(struct new_command *cmd, struct node **head) {
    pid_t spawnPid = fork();

    switch(spawnPid) {
        case -1:
            perror("Fork failed\n");
            fflush(stdout);
            break;
        case 0:
            route_input(cmd);
            route_output(cmd);
            execvp(cmd->argv[0], cmd->argv);
            perror(cmd->argv[0]);
            _exit(EXIT_FAILURE);
        default:
            printf("Adding node: %d\n", spawnPid);
            add_node(head, spawnPid);
            display_list(head);
    }
    return 1;
}

void zombie_apocalypse(struct node *head) {
    int bg_status;

    for (struct node *curr_node = head; curr_node != NULL; curr_node = curr_node->next) {
        pid_t bg_pid = waitpid(-1, &bg_status, WNOHANG);
        if (bg_pid > 0) {
            fflush(stdout);
            if (head != NULL) {
                remove_node(&head, bg_pid);
            }
            display_list(&head);

            if ((WIFEXITED(bg_status)) != 0) {
                printf("Exited PID %d with code %d\n", bg_pid, WEXITSTATUS(bg_status));
                fflush(stdout);
            } else if (WIFSIGNALED(bg_status)) {
                printf("PID %d terminated by signal %d\n", bg_pid, WTERMSIG(bg_status));
                fflush(stdout);
            }
        }

    }
}

/**
 * Takes a new_command struct as a parameter. Attempt to match argv[0] to one of the built in commands (cd, exit,
 * status), if argv[0] does not match to any of these commands, passes the command to a helper which will attempt
 * to run one of the linux commands.
 *
 * If the user calls the exit function, execute_command returns 0 which will begin exiting the shell, all other
 * commands will return 1 and continue to loop the shell.
 *
 * @param cmd - struct new_command
 */
int execute_command(struct new_command *cmd, struct node **head) {
    // If a line is blank, do nothing
    if (cmd->argc ==  0) {
        return 1;
    }
    // If the first character of a line is '#', this line is a comment, do nothing
    else if (cmd->comment == 1) {
        return 1;
    }
    // If argv[0] matches "exit", return 0 and begin exiting the shell
    else if (strcmp(cmd->argv[0], "exit") == 0) {
        return 0;
    }
    // If argv[0] matches "cd", if argc is 1, move to the HOME directory, otherwise move to the path specified in
    // argv[1]
    else if (strcmp(cmd->argv[0], "cd") == 0) {
        if (cmd->argc > 2) {
            printf("Too many arguments.\n");
            fflush(stdout);
        }
        else if (cmd->argc == 1) {
            if (chdir(getenv("HOME")) == -1) {
                printf("File: %s\n Errno: %s", getcwd(NULL, 0), strerror(errno));
                fflush(stdout);
            }
        } else {
            if (chdir(cmd->argv[1]) == -1) {
                printf( "File: %s\n Errno: %s", cmd->argv[2], strerror(errno));
                fflush(stdout);
            }
        }
        return 1;
    }
    // If argv[1] matches "status", display the exit status or the terminating signal of the last foreground
    // process run by the shell
    else if (strcmp(cmd->argv[0], "status") == 0) {
        printf("Status: %d\n", proc_status);
        fflush(stdout);
        return 1;
    }
    // argv[0] does not match any of the built-in commands, pass the cmd struct to a helper function which attempts
    // to run a native linux command
    else {
        if (cmd->bg_proc == 1) {
            fflush(stdout);
            exec_bg_cmd(cmd, head);
        } else {
            fflush(stdout);
            exec_fg_cmd(cmd);
        }
    }

    return 1;
}

void run_shell(void) {
    struct new_command *cmd;
    struct node *head = NULL;
    char *currLine = NULL;
    int running = 1;

    // Display command line prompt so user knows the shell is awaiting input
    while(running != 0) {
        if (head != NULL) {
            zombie_apocalypse(head);
        }
        printf(": ");
        fflush(stdout);
        currLine = get_line();
        cmd = parse_line(currLine);

        running = execute_command(cmd, &head);
        // clean up
        free(currLine);
        clean_up_cmd(&cmd);

    }
    // Kill bg processes
    // Free the list of bg_pid
    free_list(&head);

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
