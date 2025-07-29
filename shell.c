#include<stdio.h> 
#include<stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h> 
#include <ctype.h>
#include <string.h>

#define BUFF_SIZE  256

char** list_of_arguments;
int num_of_arguments;
char** list_of_paths;

// prints an error message to stderr
void error() {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message)); 
}

// removes leading and trailing spaces from a string
char* remove_spaces(char* string) {
    char* after; 
    if (*string == '\0') {
        return string; 
    }
    while (isspace(*string)) {
        string++;
    }
    after = string + strlen(string) - 1;
    while (isspace(*after) && after > string) {
        after--;
    }
    after[1] = '\0';    
    return string;
}

// parses a string into arguments and stores them in the global array
void store_arguments(char* string) {
    if (strlen(remove_spaces(string)) == 0) {
        return;
    }
    string = strsep(&string, ">");
    int counter = 0; 
    char* raw;
    char* cleaned;
    while((raw = strsep(&string, " ")) != NULL) {
        cleaned = remove_spaces(raw);
        if(strlen(cleaned) > 0) {
            list_of_arguments[counter] = strdup(cleaned);
            counter = counter + 1; 
        }
    }
    num_of_arguments = counter;
    free(raw);
    list_of_arguments[counter] = NULL;
}

// executes the built-in cd command to change directory
void cd_command() {
    if(num_of_arguments == 2 && chdir(list_of_arguments[1]) == 0) {
        return;
    }
    error();
}

// stores user-specified paths for executable lookup
void path_command() {
    for(int i = 1; i < num_of_arguments; i++) {
        list_of_paths[i-1] = strdup(list_of_arguments[i]);
    }
    list_of_paths[num_of_arguments - 1] = NULL;
    return;
}

char* concat_path;

// searches for the executable in the list of specified paths
int search_path() {
    int index = 0;
    while(list_of_paths[index] != NULL) { 
        concat_path = strdup(list_of_paths[index]);
        strcat(concat_path, "/");
        strcat(concat_path, list_of_arguments[0]);
        if (access(concat_path, X_OK) == 0) {
            return 0;
        }
        index = index + 1;
    }
    return -1;
}

// replaces the current process image with a new executable
void replace_address_space() {
    if (search_path() == 0) {
        list_of_arguments[0] = concat_path;
    }
    if (execv(list_of_arguments[0], list_of_arguments) == -1) {
        error();
    }
    for (int i = 0; i < num_of_arguments; i++) {
        free(list_of_arguments[i]);
    }
    free(list_of_arguments);
    exit(0);
}

// checks whether a command includes redirection
int has_redirection(char* command) {
    if (strchr(command, '>') != NULL) {
        return 1; 
    }
    return 0;
}

// handles output redirection by redirecting stdout to a specified file
void redirection(char* command) {
    strsep(&command, ">");
    if (command != NULL) {
        char* output_files = remove_spaces(command);
        char* file_name;
        int file_count = 0;
        while ((file_name = strsep(&output_files, " ")) != NULL) {
            file_name = remove_spaces(file_name);
            if (strlen(file_name) > 0) {
                file_count++; 
            }
        }
        if (file_count > 1) {
            error();
            exit(1);
        }
        if (file_count == 1) {
            char* output_file_name = remove_spaces(command);
            int output_file = open(output_file_name, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
            if (output_file == -1) {
                error();
                exit(1);
            }
            dup2(output_file, STDOUT_FILENO);
            close(output_file);
        }
    } else {
        error(); 
        exit(1);
    }
}

// executes built-in or external commands by forking a process or calling built-in logic
void create_and_execute_command(char* line) {
    line[strlen(line) - 1] = '\0';                         
    char* single_command;
    char* single_command_copy;
    while ((single_command = strsep(&line, "&")) != NULL) {
        single_command_copy = strdup(single_command);
        store_arguments(single_command);
        if (list_of_arguments[0] == NULL) {
            return;
        }
        if (strcmp(list_of_arguments[0], "cd") == 0) {
            cd_command();
            continue;
        }
        if (strcmp(list_of_arguments[0], "path") == 0) {
            path_command();
            continue; 
        }
        int status = fork();
        if (status < 0) {
            error();
            exit(1);
        }
        else if (status == 0) {
            if (has_redirection(single_command_copy) == 1) {
                redirection(single_command_copy);
            }
            replace_address_space();
        }
        else {
            continue;
        }
    } 
    while (wait(NULL) > 0);
    free(single_command);
    free(single_command_copy);
}

int main(int argc, char * argv[]) {
    list_of_arguments = (char**)malloc(BUFF_SIZE * sizeof(char*));
    list_of_paths = (char**)malloc(BUFF_SIZE * sizeof(char*));
    list_of_paths[0]= strdup("/bin");
    list_of_paths[1] = NULL;
    if (argc > 2) {
        error();
        exit(1);
    }
    else if (argc == 1) {
        char* line = NULL;
        size_t len;
        while(1) {
            printf("wish> ");
            getline(&line, &len, stdin);
            if (strcmp(line, "exit\n") == 0) {
                exit(0);
            }
            if (strcmp(line, "\n") == 0) {
                continue;
            }
            if (strcmp(line, "\t\n") == 0) {
                continue;
            }
            else {
                create_and_execute_command(line);
            }
        }
        free(line);
    }
    else {
        FILE* batch_file = fopen(argv[1], "r");
        if (batch_file == NULL) {
            error();
            exit(1);
        }
        char *lineptr = NULL;
        size_t linesize = 1024;
        while (getline(&lineptr, &linesize, batch_file) != -1) {
            if (strcmp(lineptr, "exit\n") == 0) {
                exit(0);
            }
            create_and_execute_command(lineptr);
        }
        free(lineptr);
    }
    return 0;
}
