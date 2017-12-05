#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#define MAX_EVENTS  (1024)
#define BUF_LENGTH  (128)
#define SERVER_PORT (8080)

/*
 * status: 1 表示在监听事件中，0表示不存在
 * last_active: 记录最后一次响应时间，做超市处理
 */

struct myevent_s {
    int fd;      // cfd listenfd
    int events;  // EPOLLIN EPOLLOUT
    void *arg;   // 指向自己的结构体指针
    void (*callback)(int fd, int events, void *arg);
    int status;
    char buf[BUF_LENGTH];
    int len;
    long last_active;
};

int g_efd;                                  // epoll_create 返回的句柄.
struct myevent_s g_events[MAX_EVENTS + 1];  // +1 最后一个用于listen fd.

void event_set(struct myevent_s *ev, int fd, void(*callback)(int, int, void*), void *arg) {
    ev->fd = fd;
    ev->callback = callback;
    ev->events = 0;
    ev->arg = arg;
    ev->status = 0;

    ev->last_active = time(NULL);
}

void recv_data(int fd, int events, void *arg);
void send_data(int fd, int events, void *arg);

void event_add(int efd, int events, struct myevent_s *ev) {
    struct epoll_event epv = {0, {0}};
    int op;
    epv.data.ptr = ev;
    epv.events = ev->events = events;

    if(ev->status == 1) {
        op = EPOLL_CTL_MOD;
    } else {
        op = EPOLL_CTL_ADD;
        ev->status = 1;
    }

    if(epoll_ctl(efd, op, ev->fd, &epv) < 0) {
        printf("event add failed [fd=%d], events[%d]\n", ev->fd, events);
    } else {
        printf("event add OK [fd=%d], op=%d, events[%0X]\n", ev->fd, op, events);
    }
}

void event_del(int efd, struct myevent_s *ev) {
    struct epoll_event epv = {0, {0}};

    if(ev->status != 1) {
        return;
    }

    epv.data.ptr = ev;
    ev->status = 0;
    epoll_ctl(efd, EPOLL_CTL_MOD, ev->fd, &epv);
}

void conn_accept(int lfd, int events, void* arg) {
    (void)events;
    (void)arg;
    struct sockaddr_in cin;
    socklen_t len = sizeof(cin);
    int cfd, i;

    if((cfd = accept(lfd, (struct sockaddr*)&cin, &len)) == -1) {
        if(errno != EAGAIN && errno != EINTR) {

        }

        printf("%s: accept, %s\n", __func__, strerror(errno));
        return;
    }

    do {
        for(i = 0; i < MAX_EVENTS; i++) {
            if(g_events[i].status == 0) {
                break;
            }
        }

        if(i == MAX_EVENTS) {
            printf("%s: max connect limit[%d]\n", __func__, MAX_EVENTS);
            break;
        }

        int flag = 0;
        if((flag = fcntl(cfd, F_SETFL, O_NONBLOCK)) < 0) {
            printf("%s: fcntl nonblocking failed, %s\n", __func__, strerror(errno));
            break;
        }
    } while(0);
}
