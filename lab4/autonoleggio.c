#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>
#include <errno.h>

#define MAX_CARS 256
#define MAX_NAME_LEN 256
#define STATUS_FILE "status.txt"

// Struttura per rappresentare una vettura
typedef struct {
    char name[MAX_NAME_LEN];
    int status; // 0 = free, 1 = busy
    sem_t *sem;
} Car;

Car cars[MAX_CARS];
int car_count = 0;

void read_catalog(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Cannot open catalog file");
        exit(EXIT_FAILURE);
    }

    char line[MAX_NAME_LEN];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0'; // Remove newline character
        strcpy(cars[car_count].name, line);
        car_count++;
    }

    fclose(file);
}

void read_status() {
    FILE *file = fopen(STATUS_FILE, "r");
    if (!file) {
        for (int i = 0; i < car_count; i++) {
            cars[i].status = 0; // Default to free
        }
        return;
    }

    char line[MAX_NAME_LEN + 10];
    while (fgets(line, sizeof(line), file)) {
        char name[MAX_NAME_LEN];
        int status;
        sscanf(line, "%s %d", name, &status);
        for (int i = 0; i < car_count; i++) {
            if (strcmp(cars[i].name, name) == 0) {
                cars[i].status = status;
                break;
            }
        }
    }

    fclose(file);
}

void write_status() {
    FILE *file = fopen(STATUS_FILE, "w");
    if (!file) {
        perror("Cannot open status file");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < car_count; i++) {
        fprintf(file, "%s %d\n", cars[i].name, cars[i].status);
    }

    fclose(file);
}

void init_semaphores() {
    for (int i = 0; i < car_count; i++) {
        char sem_name[MAX_NAME_LEN + 10];
        snprintf(sem_name, sizeof(sem_name), "/sem_%s", cars[i].name);
        cars[i].sem = sem_open(sem_name, O_CREAT, 0644, 1);
        if (cars[i].sem == SEM_FAILED) {
            perror("sem_open");
            exit(EXIT_FAILURE);
        }
    }
}

void close_semaphores() {
    for (int i = 0; i < car_count; i++) {
        sem_close(cars[i].sem);
    }
}

void view_cars() {
    for (int i = 0; i < car_count; i++) {
        printf("Car: %s, status: %s\n", cars[i].name, cars[i].status ? "busy" : "free");
    }
}

void lock_car(const char *car_name) {
    for (int i = 0; i < car_count; i++) {
        if (strcmp(cars[i].name, car_name) == 0) {
            sem_wait(cars[i].sem);
            if (cars[i].status == 0) {
                cars[i].status = 1;
                printf("Car: %s is now locked\n", car_name);
            } else {
                printf("Error. Car %s already locked\n", car_name);
            }
            sem_post(cars[i].sem);
            return;
        }
    }
    printf("Cannot find car %s\n", car_name);
}

void release_car(const char *car_name) {
    for (int i = 0; i < car_count; i++) {
        if (strcmp(cars[i].name, car_name) == 0) {
            sem_wait(cars[i].sem);
            if (cars[i].status == 1) {
                cars[i].status = 0;
                printf("Car: %s is now free\n", car_name);
            } else {
                printf("Error. Car %s already free\n", car_name);
            }
            sem_post(cars[i].sem);
            return;
        }
    }
    printf("Cannot find car %s\n", car_name);
}

int main() {
    read_catalog("catalog.txt");
    read_status();
    init_semaphores();

    char command[MAX_NAME_LEN + 10];
    char car_name[MAX_NAME_LEN];

    while (1) {
        printf("\033[1;31mCommand: \033[0m");
        if (fgets(command, sizeof(command), stdin) == NULL) {
            break;
        }
        command[strcspn(command, "\n")] = '\0'; // Remove newline character

        if (strcmp(command, "view") == 0) {
            view_cars();
        } else if (sscanf(command, "lock %s", car_name) == 1) {
            lock_car(car_name);
        } else if (sscanf(command, "release %s", car_name) == 1) {
            release_car(car_name);
        } else if (strcmp(command, "quit") == 0) {
            break;
        } else {
            printf("Unknown Command\n");
        }
    }

    write_status();
    close_semaphores();
    return 0;
}
