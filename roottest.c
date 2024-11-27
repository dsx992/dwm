#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <pthread.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#define MSG_SIZE            128
#define MSG_TIMEDATE_LEN    64
#define LINE_BUFFER         256

/* sunday = 0 */
char* weekdays[7] = {"Søndag", "Mandag", "Tirsdag", "Onsdag", "Torsdag", "Fredag", "Lørdag"};

pthread_mutex_t msg_lock = PTHREAD_MUTEX_INITIALIZER;

char MSG[MSG_SIZE + 1];
Display* display;
Window root;

int update_root(char* msg) {
    XStoreName(display, root, msg);
    XFlush(display);

    return 1;
}

/* 
 * msg[.offset] 
 */
int update_message(char* msg, uint32_t size, char* text) {
    printf("%c\n",msg[0]);
    printf("update_message\n");
    pthread_mutex_lock(&msg_lock);
    printf("locked\n");
    update_root(MSG);
    pthread_mutex_unlock(&msg_lock);
    printf("donez\n");
    return 1;
}


void* update_time() {
    printf("update_time\n");
    time_t t;
    struct tm* tm;
    char buf[MSG_TIMEDATE_LEN + 1];

    printf("1\n");
    tm = malloc(sizeof(struct tm));

    while(1) {
        sleep(1);
        t = time(NULL);
        tm = localtime(&t);
        memcpy(buf, weekdays[tm->tm_wday], 8);
        printf("2\n");

        pthread_mutex_lock(&msg_lock);
        sprintf(&buf[8], "%i:%i:%i",
                tm->tm_hour,
                tm->tm_min,
                tm->tm_sec);
        pthread_mutex_unlock(&msg_lock);

        printf("%i:%i:%i\n",
                tm->tm_hour,
                tm->tm_min,
                tm->tm_sec);
        update_message(&MSG[8], 10, buf);
    }

    
    printf("3\n");

    // free(tm);
    return NULL;
}

int main() {
    int screen;

    pthread_t clock_thr;

    char* line;


    if ((display = XOpenDisplay(NULL)) == NULL) {
        printf("cannot open display\n");
        return(1);
    }

    screen = DefaultScreen(display);
    root = RootWindow(display, screen);

    XStoreName(display, root, "hejsa");
    XFlush(display);
    update_root("wowsa");

    // update_root();
    pthread_create(&clock_thr, NULL, &update_time, NULL );

    // update_time();

    line = malloc(128);

    while (fgets(line, 128, stdin) != NULL) { 
        if(line[0] == 'q') {
            break;
        }
    }

    pthread_cancel(clock_thr);
    free(line);
    free(display);
    return 0;
}
