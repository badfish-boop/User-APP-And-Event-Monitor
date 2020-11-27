//关键点 TFD_TIMER_ABSTIME
//timerfd_gettime(2) - Linux man page
#include <sys/timerfd.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include <sys/time.h>
#include <unistd.h>

#include "dbus_server.h"

int systime_monitor_thread() 
{
        char front_time[200];
        char current_time[200];
        int fd = timerfd_create(CLOCK_REALTIME, 0);
        timerfd_settime(fd, TFD_TIMER_ABSTIME | TFD_TIMER_CANCEL_ON_SET,
                        &(struct itimerspec){ .it_value = { .tv_sec = INT_MAX } },
                        NULL);
        printf("Waiting\n");
        char buffer[10];
        struct timeval tv;
        while(1){
                gettimeofday (&tv, NULL);
                if (-1 == read(fd, &buffer, 10)) {
                        if (errno == ECANCELED) {
                                printf("Timer cancelled - system clock changed\n");
                                struct timeval tv_cur;
                                gettimeofday (&tv_cur, NULL);
                                printf("tv_cur:tv_sec; %ld\n", tv_cur.tv_sec);
                                printf("tv_cur：tv_usec; %ld\n", tv_cur.tv_usec);
                                printf("tv_sec; %ld\n", tv.tv_sec);
                                printf("tv_usec; %ld\n", tv.tv_usec);
                                memset(front_time,0,200);
                                memset(current_time,0,200);
                                sprintf(current_time,"%ld.%ld",tv_cur.tv_sec,tv_cur.tv_usec);
                                sprintf(front_time,"%ld.%ld",tv.tv_sec,tv.tv_usec);
                                /*
                                front_time=strdup(ctime(&tv.tv_sec));
                                current_time=strdup(ctime(&tv_cur.tv_sec));
                                */
                                printf("front_time=%s,current_time=%s\n",front_time,current_time);
                                
                                dbus_systime_change_singal_send(front_time,current_time);
                                
                        }
                        else{
                                perror("error");
                        } 
                }
        }
        close(fd);
        return 0;
}