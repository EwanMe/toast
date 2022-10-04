#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

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
        char *temp_path = malloc(128);
        temp_path[128] = '\0';

        FILE *file;

        while ( (ep = readdir(dp)) ) {
            char prefix[12];
            memcpy(prefix, &ep->d_name, 12);
            prefix[12] = '\0';

            if (strcmp(prefix, "thermal_zone") == 0) {
                char zone_path[256];
                char type_path[300];
                snprintf(zone_path, 257, "%s/%s", path, ep->d_name);
                snprintf(type_path, 301, "%s/type", zone_path);
                file = fopen(type_path, "r");

                if (file != NULL) {
                    char type[16];
                    if (fgets(type, 16, file) == NULL) printf("couldn't read\n");
                    else {
                        type[strcspn(type, "\n")] = '\0';
                        if (strcmp(type, "x86_pkg_temp") ==  0) {
                            
                            sprintf(temp_path, "%s/temp", zone_path);
                        }
                    }
                    fclose(file);
                }
            }
        }
        (void) closedir(dp);

        if (strcmp(temp_path, "") != 0) return temp_path;

        perror("Could not find CPU thermal zone.");
        exit(EXIT_FAILURE);
    }
    
    perror("Could not open directory.");
    exit(EXIT_FAILURE);
}

int main() {

    char *path = get_temp_path();
    long temp;

    while ( (temp = get_temp(path)) ) {
        printf("CPU temperature: %ld\n", temp);
        sleep(3);
    }

    free(path);

    return 0;
}
