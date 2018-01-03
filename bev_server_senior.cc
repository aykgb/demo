#include <iostream>
#include <vector>

extern "C" {
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <event.h>
#include <event2/listener.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/thread.h>
}

void listener_cb(evconnlistener *listener, evutil_socket_t fd,
        struct sockaddr *sock, int socklen, void *arg);
void socket_read_cb(bufferevent *bev, void *arg);
void socket_event_cb(bufferevent *bev, short events, void *arg);

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9999);
    addr.sin_addr.s_addr = INADDR_ANY;

    event_base *base = event_base_new();
    evconnlistener *listener = evconnlistener_new_bind(base, listener_cb, base,
            LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE,
            10, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));

    event_base_dispatch(base);
    evconnlistener_free(listener);
    event_base_free(base);

    return 0;
}

// 当一个客户端连接上服务器，libevent已经完成accept操作。
void listener_cb(evconnlistener *listener, evutil_socket_t fd, struct sockaddr *sock, int socklen, void *arg) {
    (void)listener;
    (void)sock;
    (void)socklen;
    std::cout << "accept a client." << std::endl;

    event_base* base = (event_base *)arg;

    // 为这个客户端分配一个bufferevent.
    bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(bev, socket_read_cb, NULL, socket_event_cb, NULL);
    bufferevent_enable(bev, EV_READ | EV_PERSIST);
}

void socket_read_cb(bufferevent *bev, void *arg) {
    (void)arg;
    char msg[4097];
    int len = bufferevent_read(bev, msg, sizeof(msg) - 1);
    msg[len] = '\0';

    char reply[4097];
    snprintf(reply, sizeof(reply) - 1, "%s%s", "receive message: ", msg);
    bufferevent_write(bev, reply, strlen(reply));
}

void socket_event_cb(bufferevent *bev, short events, void *arg) {
    (void)arg;
    if(events & BEV_EVENT_EOF) {
        std::cout << "connection closed." << std::endl;
    } else if (events & BEV_EVENT_ERROR) {
        std::cout << "some other error." << std::endl;
    } else {

    }

    bufferevent_free(bev);
}
