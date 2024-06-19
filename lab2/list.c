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
    printf("\tInode: %lu\n", statbuf->st_ino);

    if (S_ISREG(statbuf->st_mode)) { // regular file
        printf("\tType: file\n");
    } else if (S_ISDIR(statbuf->st_mode)) { // directory
        printf("\tType: directory\n");
    } else if (S_ISLNK(statbuf->st_mode)) { // symbolic link
        printf("\tType: symbolic link\n");
    } else if (S_ISFIFO(statbuf->st_mode)) { // FIFO
        printf("\tType: FIFO\n");
    } else {
        printf("\tType: other\n");
    }
    printf("\tSize: %ld\n", statbuf->st_size);
    printf("\tOwner: %d %s\n", statbuf->st_uid, pw->pw_name);
    printf("\tGroup: %d %s\n", statbuf->st_gid, gr->gr_name);
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

    struct stat statbuf;
    if (lstat(argv[1], &statbuf) == -1) {
        perror("lstat");
        exit(EXIT_FAILURE);
    }
    print_file_info(argv[1], &statbuf);

    traverse_directory(argv[1]);
    return 0;
}
