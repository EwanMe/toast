#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <wchar.h>
#include <locale.h>
#include "ncurses.h"

long get_temp(char *path) {
    FILE *temp_file = fopen(path, "r");
    long temp;
    fscanf(temp_file, "%ld", &temp);
    fclose(temp_file);
    return temp;
}

int get_temp_path(char *t_path, size_t p_len) {
    memset(t_path, 0x00, p_len);
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
                if (file == NULL) continue;
                
                char type[32];
                if (fgets(type, 32, file) == NULL) printf("Couldn't read from file\n");
                else {
                    type[strcspn(type, "\n")] = '\0';
                    if (strcmp(type, "x86_pkg_temp") ==  0) {
                        if (snprintf(temp_path, 512, "%s/temp", zone_path) < 0) abort();
                        strncpy(t_path, temp_path, strlen(temp_path));
                        t_path[strlen(temp_path)] = '\0';
                        fclose(file);
                        return 0;
                    }
                }
                fclose(file);
            }
        }
        (void) closedir(dp);
    }
    return -1;
}

int get_num_thermal_zones() {
    int num_zones = 0;

    DIR *dp;
    struct dirent *ep;

    const char *path = "/sys/class/thermal";
    dp = opendir(path);
    if (dp != NULL) {
        while ( (ep = readdir(dp)) ) {
            int size = 12;
            char prefix[size];
            memcpy(prefix, &ep->d_name, size);
            prefix[size] = '\0';  

            if (strcmp(prefix, "thermal_zone") == 0) {
                num_zones++;
            }
        }
    }
    return num_zones;
}

void print_header() {
    init_pair(1, COLOR_BLUE, COLOR_BLACK);

    attron(COLOR_PAIR(1));
    mvprintw(1, 1, "█─▄▄▄▄█▄─█─▄█─▄─▄─█─▄▄─█");
    mvprintw(2, 1, "█▄▄▄▄─██▄─▄████─███─██─█");
    mvprintw(3, 1, "▀▄▄▄▄▄▀▀▄▄▄▀▀▀▄▄▄▀▀▄▄▄▄▀");
    mvprintw(4, 1, "System thermal overview");
    attroff(COLOR_PAIR(1));
}

int main() {
    setlocale(LC_CTYPE, "");
    
    initscr();
    curs_set(0);

    if (!has_colors()) {
        endwin();
        perror("Your terminal does not support colors");
        exit(EXIT_FAILURE);
    }
    
    if (start_color() < 0) perror("Could not enable colors");
    
    print_header();
    
    int num_zones = get_num_thermal_zones();
    
    char *path = malloc(sizeof(char) * 512);
    if (get_temp_path(path, 512) == 0) {
        
        long temp;

        while ( (temp = get_temp(path)) ) {
            mvprintw(5, 1, "CPU temperature: %5ld", temp);
            refresh();
            sleep(1);
        }
    }

    if (path != NULL) free(path);
    curs_set(1);
    endwin();
    return 0;
}
