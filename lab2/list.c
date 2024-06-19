#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>

void print_file_info(const char *path, struct stat *statbuf) {
    struct passwd *pw = getpwuid(statbuf->st_uid);
    struct group *gr = getgrgid(statbuf->st_gid);

    printf("Node: %s\n", path);
    printf("Inode: %lu\n", statbuf->st_ino);

    if (S_ISREG(statbuf->st_mode)) {
        printf("Type: file\n");
    } else if (S_ISDIR(statbuf->st_mode)) {
        printf("Type: directory\n");
    } else if (S_ISLNK(statbuf->st_mode)) {
        printf("Type: symbolic link\n");
    } else if (S_ISFIFO(statbuf->st_mode)) {
        printf("Type: FIFO\n");
    } else {
        printf("Type: other\n");
    }

    printf("Size: %ld\n", statbuf->st_size);
    printf("Owner: %d %s\n", statbuf->st_uid, pw->pw_name);
    printf("Group: %d %s\n", statbuf->st_gid, gr->gr_name);
}

void traverse_directory(const char *dir_path) {
    DIR *dir = opendir(dir_path);
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    char path[1024];

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);

        struct stat statbuf;
        if (lstat(path, &statbuf) == -1) {
            perror("lstat");
            continue;
        }

        print_file_info(path, &statbuf);

        if (S_ISDIR(statbuf.st_mode)) {
            traverse_directory(path);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <directory>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    traverse_directory(argv[1]);

    return 0;
}
