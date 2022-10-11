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

    char **paths = malloc(num_zones * sizeof(char *));
    char **types = malloc(num_zones * sizeof(char *));

    const char *base_p = "/sys/class/thermal/thermal_zone";
    
    for (int i = 0; i < num_zones; ++i) {
        int path_len = strlen(base_p) + strlen("/temp") + 3;
        paths[i] = malloc(path_len * sizeof (char));
        snprintf(paths[i], path_len, "%s%d/temp", base_p, i);
        
        char *type_p = malloc(path_len * sizeof (char));
        snprintf(type_p, path_len, "%s%d/type", base_p, i);

        FILE *f = fopen(type_p, "r");
        
        types[i] = malloc(64);
        memset(types[i], 0x00, 64);
        fscanf(f, "%s", types[i]);
        fclose(f);
    }

    while (1) {
        for (int i = 0; i < num_zones; ++i) {
            long temp = get_temp(paths[i]);
            mvprintw(5 + i, 1, "%s: %5ld", types[i], temp);
            refresh();
        }
        sleep(1);
    }

    for (int i = 0; i < num_zones; ++i) {
        free(paths[i]);
    }
    free(paths);

    for (int i = 0; i < num_zones; ++i) {
        free(types[i]);
    }
    free(types);

    curs_set(1);
    endwin();
    return 0;
}
