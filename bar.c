#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <pthread.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
// #include <signal.h>
#include <unistd.h>
#include <limits.h>

#define BAT0_CAP            "/sys/class/power_supply/BAT0/capacity"
#define BAT0_STATUS         "/sys/class/power_supply/BAT0/status"

#define MSG_SIZE            64

#define TIMEDATE_LEN        20
#define BATTERY_LEN         5
#define AUDIO_LEN

#define MSG_BAT_START       TIMEDATE_LEN
#define LINE_BUFFER         256

/* sunday = 0 */
char* weekdays[7] = {"søndag", "mandag", "tirsdag", "onsdag", "torsdag", "fredag", "lørdag"};

pthread_mutex_t msg_lock = PTHREAD_MUTEX_INITIALIZER;

char status[MSG_SIZE + 1];

Display* display;
Window root;

int update_root(char* msg) {
    XStoreName(display, root, msg);
    XFlush(display);

    return 1;
}


int update_status(size_t from, size_t length, char* msg) {
    pthread_mutex_lock(&msg_lock);
    memcpy(&status[from], msg, length);

    status[MSG_SIZE] = '\0';        // safety
    XStoreName(display, root, status);
    XFlush(display);

    pthread_mutex_unlock(&msg_lock);
    return 1;
}

void* update_battery() {
    FILE* fp;
    char cap[4];
    char stat[16];
    char msg[BATTERY_LEN];
    size_t msg_len;

    while(1) {
        if ((fp = fopen(BAT0_CAP, "r")) == NULL) {
            return NULL;
        }

        fgets(cap, 4, fp);
        cap[strlen(cap) - 1] = '\0';
        fclose(fp);
        if ((fp = fopen(BAT0_STATUS, "r")) == NULL) {
            return NULL;
        }
        fgets(stat, 16, fp);
        stat[strlen(stat) - 1] = '\0';
        fclose(fp);

        // TODO: optimize
        if (strncmp(stat, "Discharging", 10) == 0) {
            stat[0] = '\0';
        } else {
            stat[0] = '+';
            stat[1] = '\0';
        }

        msg_len = sprintf(msg, "%s%s%%", stat, cap);
        if (msg_len < BATTERY_LEN) {
            memset(&msg[msg_len], ' ', BATTERY_LEN - msg_len);
        }

        update_status(MSG_BAT_START, BATTERY_LEN, msg);
        sleep(5);
    }

    return NULL;

}

void* update_time() {
    char msg[TIMEDATE_LEN + 1];
    time_t t;
    struct tm* tm;
    size_t msg_len;

    tm = malloc(sizeof(struct tm));

    while(1) {
        t = time(NULL);
        tm = localtime(&t);

        // memcpy(msg, weekdays[tm->tm_wday], 8);
        msg_len = sprintf(msg, "%s %2.2i:%2.2i:%2.2i",
                weekdays[tm->tm_wday],
                tm->tm_hour,
                tm->tm_min,
                tm->tm_sec);
        if (msg_len < TIMEDATE_LEN) {
            memset(&msg[msg_len], ' ', TIMEDATE_LEN - msg_len);
        }
        
        update_status(0, TIMEDATE_LEN, msg);
        sleep(1);
    }
    // free(tm);
    return NULL;
}

int main() {
    int screen;

    pthread_t clock_thr;
    pthread_t bat_thr;

    char* line;

    if ((display = XOpenDisplay(NULL)) == NULL) {
        printf("cannot open display\n");
        return(1);
    }

    screen = DefaultScreen(display);
    root = RootWindow(display, screen);

    pthread_create(&clock_thr, NULL, &update_time, NULL );
    pthread_create(&bat_thr, NULL, &update_battery, NULL );

    line = malloc(128);

    while (1) {
        sleep(UINT_MAX);
    }

    pthread_cancel(clock_thr);
    pthread_cancel(bat_thr);
    free(line);
    free(display);
    return 0;
}
