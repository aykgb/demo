#include <iostream>
#include <vector>
#include <string>

using namespace std;

extern "C" {
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <event.h>
#include <event2/bufferevent.h>
}

void accept_cb(int fd, short events, void *arg);
void socket_read_cb(struct bufferevent *bev, void *arg);
void event_cb(struct bufferevent *bev, short events, void *arg);
int tcp_server_init(int port, int listen_num);

int main()
{
    // 初始化一个listenfd
    int listenfd = tcp_server_init(9999, 10);
    if(listenfd == -1) {
        std::cerr << "tcp_server_init error." << std::endl;
        return -1;
    }

    // new 一个event_base
    struct event_base *base = event_base_new();

    // 在base上添加一个listen事件
    struct event *ev_listen = event_new(base, listenfd, EV_READ | EV_PERSIST, accept_cb, base);
    event_add(ev_listen, NULL);

    event_base_dispatch(base);
    event_base_free(base);

    return 0;
}

void accept_cb(int fd, short events, void *arg) {
    (void)events;

    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);

    evutil_socket_t acceptfd = ::accept(fd, (struct sockaddr *)&client_addr, &len);
    evutil_make_socket_nonblocking(acceptfd);

    std::cout << "accept a client.fd: " << acceptfd << std::endl;

    struct event_base *base = (struct event_base *)arg;
    struct bufferevent *bev = bufferevent_socket_new(base, acceptfd, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(bev, socket_read_cb, NULL, event_cb, arg);

    bufferevent_enable(bev, EV_READ | EV_PERSIST);
}

void socket_read_cb(struct bufferevent *bev, void *arg) {
    (void)arg;
    char msg[4097];

    int len = bufferevent_read(bev, msg, sizeof(msg) - 1);
    msg[len] = '\0';

    std::cout << "message received from client: " << msg << std::endl;

    char reply[4097];
    snprintf(reply, sizeof(reply), "%s%d", "I have received message from you. length of received msg:", len);

    // bufferevent_write(bev, reply, sizeof(reply)); //  错误写法，导致reply消息太长，在客户端侧显示有问题。
    len = bufferevent_write(bev, reply, strlen(reply));
    std::cout << "message write len: " << len << std::endl;
}

void event_cb(struct bufferevent *bev, short events, void *arg) {
    (void)arg;
    if(events & BEV_EVENT_EOF) {
        std::cerr << "connection closed." << std::endl;
    } else if (events & BEV_EVENT_ERROR) {
        std::cerr << "some other error." << std::endl;
    } else {

    }

    bufferevent_free(bev);
}

int tcp_server_init(int port, int listen_num) {
    struct sockaddr_in listen_addr;
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = INADDR_ANY;
    listen_addr.sin_port = htons(port);

    int listenfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    evutil_make_listen_socket_reuseable(listenfd);

    if(::bind(listenfd, (struct sockaddr *)&listen_addr, sizeof(listen_addr)) < 0) {
        goto error;
    }

    if(::listen(listenfd, listen_num) < 0) {
        goto error;
    }

    evutil_make_socket_nonblocking(listenfd);

    return listenfd;

error:
    int errno_save = errno;
    close(listenfd);
    errno = errno_save;

    return -1;
}
