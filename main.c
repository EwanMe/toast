#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "ncurses.h"

long get_temp(char *path) {
    FILE *temp_file = fopen(path, "r");
    long temp;
    fscanf(temp_file, "%ld", &temp);
    fclose(temp_file);
    return temp;
}

char* get_temp_path() {
    DIR *dp;
    struct dirent *ep;

    const char *path = "/sys/class/thermal";
    dp = opendir(path);
    if (dp != NULL) {
        char *temp_path = malloc(512);
        temp_path[512] = '\0';

        FILE *file;

        while ( (ep = readdir(dp)) ) {
            char prefix[12];
            memcpy(prefix, &ep->d_name, 12);
            prefix[12] = '\0';

            if (strcmp(prefix, "thermal_zone") == 0) {
                char zone_path[128];
                char type_path[255];
                if (snprintf(zone_path, 128, "%s/%s", path, ep->d_name) < 0) abort();
                if (snprintf(type_path, 255, "%s/type", zone_path) < 0) abort();
                file = fopen(type_path, "r");

                if (file != NULL) {
                    char type[32];
                    if (fgets(type, 32, file) == NULL) printf("couldn't read\n");
                    else {
                        type[strcspn(type, "\n")] = '\0';
                        if (strcmp(type, "x86_pkg_temp") ==  0) {
                            
                            if (snprintf(temp_path, 512, "%s/temp", zone_path) < 0) abort();
                        }
                    }
                    fclose(file);
                }
            }
        }
        (void) closedir(dp);

        if (strcmp(temp_path, "") != 0) return temp_path;

        perror("Could not find CPU thermal zone");
        exit(EXIT_FAILURE);
    }
    
    perror("Could not open directory");
    exit(EXIT_FAILURE);
}

int main() {
    initscr();
    char *path = get_temp_path();
    long temp;

    while ( (temp = get_temp(path)) ) {
        mvprintw(0, 0, "CPU temperature: %5ld", temp);
        refresh();
        sleep(1);
    }

    free(path);

    endwin();
    return 0;
}
