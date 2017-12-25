extern "C" {
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <event.h>
#include <event2/util.h>
}

#include <iostream>
#include <string>
#include <vector>

using namespace std;

int tcp_connect_server(const char *server_ip, int port);
void terminal_msg_cb(int fd, short events, void * arg);
void socket_read_cb(int fd, short events, void * arg);

int main(int argc, char* argv[])
{
    if(argc != 3) {
        std::cout << "Need 2 parameters of ip and port.\n";
    }

    int sockfd = tcp_connect_server(argv[1], atoi(argv[2]));
    if(sockfd == -1) {
        std::cout << "connect to server failed.\n";
        exit(-1);
    } else {
        std::cout << "connect to server successfully.\n";
    }

    struct event_base * base = event_base_new();
    struct event *ev_sockfd, *ev_terminal;
    ev_sockfd = event_new(base, sockfd,
            EV_READ | EV_PERSIST, socket_read_cb, NULL);
    ev_terminal = event_new(base, STDIN_FILENO,
            EV_READ | EV_PERSIST, terminal_msg_cb, (void*)&sockfd);

    event_add(ev_sockfd, NULL);
    event_add(ev_terminal, NULL);

    event_base_dispatch(base);

    std::cout << "finish." << std::endl;

    return 0;
}

int tcp_connect_server(const char *server_ip, int port) {
    /* Initialize server sockaddr. */
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htonl(port);
    if(0 == inet_aton(server_ip, &server_addr.sin_addr)) {
        errno = EINVAL;
        std::cout << "invalid ip address.\n";
        return -1;
    }

    /* new a socket fd. */
    int sockfd = ::socket(PF_INET, SOCK_STREAM, 0);
    if(-1 == sockfd) {
        std::cout << "create sockfd failed.\n";
        return -1;
    }

    /* connect the fd. */
    if(-1 == ::connect(sockfd,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr))) {
        std::cout << "connect sockfd failed. no server is in serving on ";
        std::cout << "ip: " << server_ip << " port: " << port << "\n";
        ::close(sockfd);
        return -1;
    }

    /* make fd nonblocking. */
    evutil_make_socket_nonblocking(sockfd);

    return sockfd;
}

void socket_read_cb(int fd, short events, void* arg) {
    (void)events;
    (void)arg;
    char msg[1024];

    // 不考虑读一半的情况。
    int len = read(fd, msg, sizeof(msg) - 1);
    if(len < 0) {
        std::cout << "read data from fd failed. fd:" << fd << "\n";
        exit(-1);
    }

    msg[len] = '\0';

    std::cout << "Message is received. len: " << len
              << " msg: " << msg << std::endl;
}

void terminal_msg_cb(int fd, short events, void* arg) {
    (void)events;
    char msg[1024];

    int sockfd = *((int *)arg);
    int len = read(fd, msg, sizeof(msg) - 1);
    if(len < 0) {
        std::cout << "read data from fd failed. fd:" << fd << "\n";
        exit(-1);
    }


    // 不考虑写一半的情况。
    int w_len = write(sockfd, msg, len);
    if(w_len < len) {
        std::cout << "received data do not write to server completely.\n";
    }
}

