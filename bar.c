#include <stdlib.h>
#include<sys/inotify.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>

#define BAT_C "/sys/class/power_supply/BAT0/capacity"
// #define BAT_C "test"

typedef struct inotify_event event_t;
size_t event_s = sizeof(event_t);

int readEvent(int fd, event_t* e) {
    if (read(fd, e, event_s) < 0) {
        return 0;
    }
    read(fd, (char*)e + event_s, e->len);
    return 1;
}

int main() {
    int fd;     // watch
    int wd;
    FILE* f;
    event_t* e; // event
    char bat_c[4];

    e = malloc(event_s + PATH_MAX);

    fd = inotify_init();
    wd = inotify_add_watch(fd, BAT_C, IN_MOVE_SELF | 
                               IN_MODIFY | 
                               IN_CLOSE_WRITE);

    while(readEvent(fd, e)) {
        printf("event: %i, %x, %u, %u \n", e->wd, e->mask, e->cookie,e->len);
        if(e->mask & IN_MOVE_SELF) {
            inotify_rm_watch(fd, wd);
            wd = inotify_add_watch(fd, BAT_C,IN_MOVE_SELF | 
                                       IN_MODIFY | 
                                       IN_CLOSE_WRITE);
        }
        if ((f = fopen(BAT_C, "r")) != NULL) {
            for(size_t i = fread(bat_c, 1, 4, f); i < 4; i++) {
                if (i < 0)
                    break;
                printf("%zu\n", i);
                bat_c[i] = '\0';
            }
            fclose(f);
            for(int i = 0; i < 4; i++) {
                printf("%c", bat_c[i]);
            }
            printf("\n");
        }
    }

    free(e);
}

