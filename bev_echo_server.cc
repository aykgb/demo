#include <stdio.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

#include <event2/event.h>
#include <event2/bufferevent.h>

#define LISTEN_PORT    9999
#define LISTEN_BACKLOG 32

void do_accept(evutil_socket_t listener, short event, void *arg);
void read_cb(struct bufferevent *bev, void *arg);
void error_cb(struct bufferevent *bev, int short, void *arg);
void wirte_cb(struct bufferevent * bev, void *arg);

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    int ret = 0;
    evutil_socket_t listener;
    listener = socket(AF_INET, SOCK_STREAM, 0);
    assert(listener > = 0);
    evutil_make_listen_socket_reuseable(listener);

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = 0;
    sin.sin_port = htons(LISTEN_PORT);

    ret = bind(listener, (struct sockaddr *)&sin, sizeof(sin));
    if(ret < 0) {
        perror("bind");
        return ret;
    }

    ret = listen(listener, LISTEN_BACKLOG);
    if( ret < 0) {
        perror("listen");
        return ret;
    }

    printf("listening ...\n");
    evutil_make_socket_nonblocking(listener);

    struct event_base * base = event_base_new();
    assert(base != NULL);
    struct event * listen_event;
    listen_event = event_new(base, listener, EV_READ | EV_PERSIST, do_accept, (void*)base);
    event_add(listen_event, NULL);
    event_base_dispatch(base);

    printf("The End.");
    return ret;
}

void do_accept(evutil_socket_t listener, short event, void *arg)  {
    (void)event;
    struct event_base *base = (struct event_base*)arg;
    struct sockaddr_in sin;
    socklen_t slen;
    evutil_socket_t fd = accept(listener, (struct sockaddr*)&sin, &slen);
    if(fd < 0) {
        perror("accept");
        return;
    }
    if(fd > FD_SETSIZE) {
        perror("fd > FD_SETSIZE\n");
        return;
    }

    printf("ACCEPT: fd = %u\n", fd);
    struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(bev, read_cb, NULL, error_cb, arg);
    bufferevent_enable(bev, EV_READ | EV_WRITE | EV_PERSIST);
}

void read_cb(struct bufferevent *bev, void *arg) {
#define MAX_LINE 256
    (void)arg;
    char line[MAX_LINE + 1];
    int n;
    evutil_socket_t fd = bufferevent_getfd(bev);
    while(n = bufferevent_read(bev, line, MAX_LINE), n > 0) {
        line[n] = '\0';
        printf("fd = %u, read line: %s\n", fd, line);

        bufferevent_write(bev, line, n);
    }

}

void write_cb(struct bufferevent *bev, void *arg) {
    (void)bev;
    (void)arg;
}

void error_cb(struct bufferevent *bev, short event, void *) {
    evutil_socket_t fd = bufferevent_getfd(bev);
    printf("fd = %u, ", fd);
    if(event & BEV_EVENT_TIMEOUT) {
        printf("Timed out.\n"); // if bufferevent_set_timeouts() called
    } else if( event & BEV_EVENT_EOF) {
        printf("connection closed.\n");
    } else if (event & BEV_EVENT_ERROR) {
        printf("some other error.\n");
    } else {
        printf("Unknow error.\n");
    }

    bufferevent_free(bev);
}
