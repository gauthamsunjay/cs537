#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>

int parseargs(char*, int*, char***, char**, int*);
void cleanup(int, char***, char**, int*);
int isbuiltin(char*);
void redirect(int);
void printerr();
int execbuiltin(int, int, char**);
void execute_cmd(char**, char*);
int execute(FILE*, char*);

int num_paths = 0;
char** paths;
char* builtins[] = {"exit", "cd", "path"};
char error_message[30] = "An error has occurred\n";

int main(int argc, char* argv[]){
    char* prompt = "wish>";
    FILE* fp = stdin;
    paths = (char **) calloc(1, sizeof(char *));
    paths[num_paths] = "/bin/";
    num_paths++;

    if (argc > 1) {
        if (argc > 2) {
            printerr();
            exit(1);
        }

        fp = fopen(argv[1], "r");
        if (fp == NULL) {
            printerr();
            exit(1);
        }
        execute(fp, "batch");
        fclose(fp);
        return 0;
    }

    while (1) {
        printf("%s ", prompt);
        fflush(stdout);
        if (execute(fp, "int") == -1) {
            return 0;
        }
    }

    return 0;
}

void printerr() {
    write(STDERR_FILENO, error_message, strlen(error_message));
}

void redirect(int fd) {
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);
}

int isbuiltin(char* cmd) {
    int i;
    for (i = 0; i < 3; i++) {
        if (strcmp(cmd, builtins[i]) == 0) {
            return i;
        }
    }
    return -1;
}


int parseargs(char* line, int* num_cmds, char*** cmds, char** filenames, int* args_count) {
    char *cmd;
    char *arg;
    char *argl;

    while ((cmd = strtok_r(line, "&\n", &line))) {
        if (*num_cmds > 0 && *num_cmds % 10 == 0) {
            cmds = realloc(cmds, (*num_cmds + 10) * sizeof(char**));
            filenames = realloc(filenames, (*num_cmds + 10) * sizeof(char*));
            args_count = realloc(args_count, (*num_cmds + 10) * sizeof(int));
        }

        cmds[*num_cmds] = (char **) malloc(10 * sizeof(char*));

        int num_args = 0;
        char* first = strchr(cmd, '>');
        char* last = strrchr(cmd, '>');

        if (first != NULL && last != NULL && first != last) {
            return -1;
        }

        int i = 0;
        int file_passed = 0;
        char* contains_redirection = strstr(cmd, ">");

        while ((argl = strtok_r(cmd, ">", &cmd))) {
            if (i == 0) {
                filenames[*num_cmds] = (char *) calloc(2, sizeof(char));
                strncpy(filenames[*num_cmds], "#", strlen("#"));
                while ((arg = strtok_r(argl, " \t\n", &argl))) {
                    if (num_args > 0 && num_args % 10 == 0) {
                        cmds[*num_cmds] = realloc(cmds[*num_cmds], (num_args + 10) * sizeof(char*));
                    }
                    cmds[*num_cmds][num_args] = (char *) calloc(strlen(arg) + 1, sizeof(char));
                    strncpy(cmds[*num_cmds][num_args], arg, strlen(arg));
                    num_args++;
                }
                cmds[*num_cmds][num_args] = NULL;
            }
            
            else {
                while ((arg = strtok_r(argl, " \t\n", &argl))) {
                    file_passed++;
                    filenames[*num_cmds] = realloc(filenames[*num_cmds], strlen(arg) + 1);
                    strncpy(filenames[*num_cmds], arg, strlen(arg));
                }
                
                if (file_passed > 1)
                    return -1;
            }
            i++;
        }
        
        if (contains_redirection !=NULL && file_passed == 0)
            return -1;

        *(args_count + *(num_cmds)) = num_args;
        (*num_cmds)++;
    }
    return 0;
}

void cleanup(int num_cmds, char*** cmds, char** filenames, int* args_count) {
    int i;
    for (i = 0; i < num_cmds; i++) {
        int j;
        for (j = 0; j < args_count[i]; j++) {
            if (cmds[i][j] != NULL)
                free(cmds[i][j]);
        }
        free(cmds[i]);
        free(filenames[i]);
    }
    free(cmds);
    free(args_count);
    free(filenames);
}

int execbuiltin(int index, int argc, char** args) {
    int error = 0;
    switch (index) {
        case 0:
            if (argc > 1) {
                printerr(); 
                break;
            }
            exit(0);

        case 1: 
            if (argc != 2)
                error = 1;

            else if(chdir(args[1]) == -1) {
                printf("Changing to %s\n", args[1]);
                error = 1;
            }
            
            if (error)
                printerr();
            break;

        case 2:
            free(paths);
            num_paths = argc - 1;
            paths = (char **) calloc(num_paths, sizeof(char *));
            if (num_paths > 0) {
                int i;    
                for (i = 1; i < argc; i++) {
                    int n = strlen(args[i]);
                    char* arg = (char *) malloc(n);
                    paths[i - 1] = strncpy(arg, args[i], n);
                    if (arg[n - 1] != '/')
                        arg = strcat(arg, "/");
                }
            }
            break;

        default:
            printerr();
    }
    return 0;
}

void execute_cmd(char** args, char* filename) {
    int rc = fork();
    if (rc == 0) {
        int i;
        char* path;
        char* command;

        if (strcmp(filename, "#") != 0) {
            mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
            redirect(open(filename, O_CREAT | O_TRUNC | O_WRONLY, mode));
        }

        for (i = 0; i < num_paths; i++) {
            path = (char *) calloc(strlen(paths[i]) + 1, sizeof(char));
            strncpy(path, paths[i], strlen(paths[i]));
            command = strcat(path, args[0]);
            if (access(command, X_OK) == 0) {
                execv(command, args);
            }
        }

        printerr();
        exit(0);
    }
}


int execute(FILE* fp, char* mode) {
    int i;
    char* line = NULL;
    size_t linecap = 0;
    ssize_t linelen;

    int is_int = strcmp(mode, "int");

    while ((linelen = getline(&line, &linecap, fp)) != -1) {
        if (linelen <= 1)
            break;

        int num_cmds = 0;
        char*** cmds = (char***) malloc(10 * sizeof(char**));
        char** filenames = (char**) malloc(10 * sizeof(char*));
        int* args_count = (int*) malloc(10 * sizeof(int));
        
        if (parseargs(line, &num_cmds, cmds, filenames, args_count) == -1) {
            cleanup(num_cmds, cmds, filenames, args_count);
            printerr();
            break;
        }

        if (num_cmds == 0) {
            cleanup(num_cmds, cmds, filenames, args_count);
            if (is_int == 0)
                break;
            continue;
        }

        int num_children = 0;
        for (i = 0; i < num_cmds; i++) {
            if (args_count[i] > 0) {
                int builtin_no = isbuiltin(cmds[i][0]);
                if (builtin_no > -1) {
                    execbuiltin(builtin_no, args_count[i], cmds[i]);
                }
                else {
                    num_children++;
                    execute_cmd(cmds[i], filenames[i]);
                }
            }
        }

        for (i = 0; i < num_children; i++) {
            wait(NULL);
        }

        cleanup(num_cmds, cmds, filenames, args_count);
        if (is_int == 0)
            break;

    }

    return linelen;
}
