#include <iostream>
#include <string>

using namespace std;

extern "C" {
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>
#include <unistd.h>

#include <event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/util.h>
}

int tcp_server_connect(const char *server_ip, int port);

void cmd_msg_cb(int fd, short events, void *arg);
void server_msg_cb(struct bufferevent *bev, void *arg);
void event_cb(struct bufferevent *bev, short events, void *arg);

int main(int argc, char *argv[])
{
    if(argc != 3) {
        std::cerr << "parameters must be 2. ip&port." << std::endl;
        exit(-1);
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2]));
    int status = inet_aton(argv[1], &addr.sin_addr);
    if(status == 0) {
        std::cerr << "server ip is not valid value." << std::endl;
        errno = EINVAL;
        return -1;
    }

    // int sockfd = tcp_server_connect(argv[1], atoi(argv[2]));
    struct event_base *base = event_base_new();
    struct bufferevent *bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
    struct event *ev_cmd = event_new(base, STDIN_FILENO, EV_READ | EV_PERSIST, cmd_msg_cb, (void*)bev);
    event_add(ev_cmd, NULL);

    bufferevent_socket_connect(bev, (struct sockaddr *)&addr, sizeof(addr));
    bufferevent_setcb(bev, server_msg_cb, NULL, event_cb, (void *)ev_cmd);
    bufferevent_enable(bev, EV_READ | EV_PERSIST);

    event_base_dispatch(base);
    std::cout << "finished." << std::endl;

    return 0;
}

void cmd_msg_cb(int fd, short events, void *arg) {
    (void)events;
    char cli_msg[4097];
    int len = read(fd, cli_msg, sizeof(cli_msg));
    if(len < 0) {
        std::cerr << "read from command line error." << std::endl;
        exit(-1);
    }
    cli_msg[len] = '\0';

    struct bufferevent *bev = (struct bufferevent *)arg;
    bufferevent_write(bev, cli_msg, len);

}

void server_msg_cb(struct bufferevent *bev, void *arg) {
    (void)arg;
    char msg[4097];

    size_t len = bufferevent_read(bev, msg, sizeof(msg) - 1);
    msg[len] = '\0';

    std::cout << "recv msg from server: " << msg << std::endl;
}

void event_cb(struct bufferevent *bev, short events, void*arg) {
    if(events & BEV_EVENT_EOF) {
        std::cerr << "connection closed." << std::endl;
    } else if( events & BEV_EVENT_ERROR ) {
        std::cerr << "some other error." << std::endl;
    } else if( events & BEV_EVENT_CONNECTED ) {
        std::cerr << "the client has connected to server." << std::endl;
        return;
    } else {

    }

    bufferevent_free(bev);

    struct event *ev = (struct event *)arg;
    event_free(ev);
}

int tcp_server_connect(const char *server_ip, int port) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    int status = inet_aton(server_ip, &addr.sin_addr);
    if(status == 0) {
        std::cerr << "server ip is not valid value." << std::endl;
        errno = EINVAL;
        return -1;
    }

    int sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sockfd == -1) {
        std::cerr << "socket creation is failed." << std::endl;
        return -1;
    }

    status = ::connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));
    if(status == -1) {
        int save_errno = errno;
        close(sockfd);
        errno = save_errno;
        return -1;
    }

    evutil_make_socket_nonblocking(sockfd);
    return sockfd;
}
