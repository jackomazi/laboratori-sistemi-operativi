#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAXPROCESSES 256
#define MAXCMD 256

void replace_percent(char *command, const char *param, char *result) {
    char *pos = strstr(command, "%");
    if (pos) {
        strncpy(result, command, pos - command);
        result[pos - command] = '\0';
        strcat(result, param);
        strcat(result, pos + 1);
    } else {
        strcpy(result, command);
    }
}

void execute_commands(int read_fd) {
    char command[MAXCMD];
    while (read(read_fd, command, MAXCMD) > 0) {
        system(command);
    }
    close(read_fd);
}


int main(int argc, char *argv[]){
    printf("argc: %d\n", argc);
    if (argc != 4){
        printf("Usage: %s <file> <num_processes> <command> \n", argv[0]);
        exit(1);
    }

    char *filename = argv[1];
    int num_processes = atoi(argv[2]);
    char *base_command = argv[3];

    if (num_processes > MAXPROCESSES) {
        fprintf(stderr, "Number of processes exceeds MAXPROCESSES\n");
        exit(1);
    }

    FILE *file = fopen(filename, "r");
    if (file == NULL){
        fprintf(stderr, "Error opening file\n");
        exit(1);
    }

    int pipes[MAXPROCESSES][2];

    for (int i = 0; i < num_processes; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(1);
        }
    }

    for (int i = 0; i < num_processes; i++) {
        if (fork() == 0) {
            for (int j = 0; j < num_processes; j++) {
                if (j != i) close(pipes[j][0]);
                close(pipes[j][1]);
            }
            execute_commands(pipes[i][0]);
            exit(EXIT_SUCCESS);
        }
    }

    char param[MAXCMD];
    int current_process = 0;
    while (fgets(param, MAXCMD, file)) {
        param[strcspn(param, "\n")] = '\0';
        char command[MAXCMD];
        replace_percent(base_command, param, command);
        write(pipes[current_process][1], command, MAXCMD);
        current_process = (current_process + 1) % num_processes;
    }
    fclose(file);

    for (int i = 0; i < num_processes; i++) {
        close(pipes[i][1]);
    }

    for (int i = 0; i < num_processes; i++) {
        wait(NULL);
    }

    return 0;

}