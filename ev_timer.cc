#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <event.h>

void on_time(int sock, short event, void *arg) {
    (void)sock;
    (void)event;
    printf("Hello, libevent world.\n");

    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    // 重新添加定时事件(定时事件触发后自动删除)
    event_add((struct event*)arg, &tv);
}

int main()
{
    event_init();

    struct event ev_time;

    evtimer_set(&ev_time, on_time, &ev_time);

    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    // 添加定时事件
    event_add(&ev_time, &tv);

    // 事件循环
    event_dispatch();

    return 0;
}
