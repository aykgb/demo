extern "C" {
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <event.h>
}

#include <iostream>
#include <string>
#include <vector>

using namespace std;

void accept_cb(int fd, short events, void *arg);
void socket_read_cb(int fd, short events, void *arg);

int tcp_server_init(int port, int listen_num);

int main(int argc, char * argv[])
{
    if(argc != 2) {
        std::cout << "Wrong parameter number, must be two.\n"
                  << "Usage: >./server.out [port]\n";
        exit(-1);
    }

    int port = atoi(argv[1]);
    int lfd = tcp_server_init(port, 20);

    struct event_base *base = event_base_new();

    // 添加listen事件
    struct event *listen_event = event_new(base, lfd, EV_READ | EV_PERSIST, accept_cb, base);
    event_add(listen_event, NULL);

    event_base_dispatch(base);

    return 0;
}


#define ERROR_HANDLE(fd) \
    int errno_save = errno;  \
    evutil_closesocket(fd);  \
    errno = errno_save;      \
    return -1;

int tcp_server_init(int port, int listen_num) {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    evutil_socket_t lfd = ::socket(AF_INET, SOCK_STREAM, 0);
    if(lfd == -1) {
        std::cout << "error: create sockfd failed.\n";
        ERROR_HANDLE(lfd);
    }

    if(::bind(lfd, (struct sockaddr *)&addr, sizeof(addr))) {
        std::cout << "error: bind failed.\n";
        ERROR_HANDLE(lfd);
    }

    if(::listen(lfd, listen_num)) {
        std::cout << "error: listen failed.\n";
        ERROR_HANDLE(lfd);
    }

    evutil_make_socket_nonblocking(lfd);

    return lfd;
}

void accept_cb(int fd, short events, void* arg) {
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    evutil_socket_t cfd = ::accept(fd, (struct sockaddr *)&client_addr, &len);

    evutil_make_socket_nonblocking(cfd);

    std::cout << "accept a client connection." << std::endl;

    struct event_base *base = (struct event_base*)arg;
    struct event *ev = event_new(NULL, -1, 0, NULL, NULL);
    event_assign(ev, base, cfd, EV_READ | EV_PERSIST, socket_read_cb, (void*)ev);

    event_add(ev, NULL);
}

void socket_read_cb(int fd, short events, void *arg) {
    char msg[4096];
    struct event *ev = (struct event *)arg;

    int len = read(fd, msg, sizeof(msg) - 1);
    if(len < 0) {
        std::cout << "some error happen when read from socket.\n";
        event_free(ev);
        close(fd);
        return;
    }

    msg[len] = '\0';
    std::cout << "receive msg: " << msg << '\0';

    char reply_msg[4096] = "I have received message from you.";
    strcat(reply_msg + strlen(reply_msg), msg);

    write(fd, reply_msg, strlen(reply_msg));
}
