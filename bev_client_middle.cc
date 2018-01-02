#include <iostream>
#include <vector>
#include <string>

using namespace std;

extern "C" {
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

#include <event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>
}

int tcp_connect_server(const char * server_ip, int port);

void cmd_msg_cb(int fd, short events, void* arg);
void server_msg_cb(struct bufferevent *bev, void *arg);
void event_cb(struct bufferevent *bev, short event, void *arg);

// 原理：在stdin上添加一个read event，将命令行上的数据写到server，然后接收并打印返回消息
// 工作流程：
// 1. stdin上有可读数据，stdin上的可读事件触发，调用cmd_msg_cb
// 2. 当stdin上有数据可读，调用cmd_msg_cb函数
int main(int argc, char *argv[]) {
    if(argc != 3) {
        std::cout << "must input two parameters. ip&port." << std::endl;;
        return -1;
    }

    int sockfd = tcp_connect_server(argv[1], atoi(argv[2]));
    if(sockfd == -1) {
        std::cerr << "tcp_connect_server failed." << std::endl;
        return -1;
    }

    struct event_base *base = event_base_new();
    struct bufferevent *bev = bufferevent_socket_new(base, sockfd,  BEV_OPT_CLOSE_ON_FREE);

    struct event *ev_cmd = event_new(base, STDIN_FILENO, EV_READ | EV_PERSIST, cmd_msg_cb, (void *)bev);

    event_add(ev_cmd, NULL);
    // args: bufferevent, read_cb, write_cb, event_cb, arg
    bufferevent_setcb(bev, server_msg_cb, NULL, event_cb, (void *)ev_cmd);
    bufferevent_enable(bev, EV_READ | EV_PERSIST);

    event_base_dispatch(base);

    std::cout << "finished." << std::endl;

    return 0;
}

// 把从命令行上收到的数据写到传入的bufferevent上事件上触发
// 这个bufferevent是连接在server的sockfd之上的
void cmd_msg_cb(int fd, short events, void *arg) {
    (void)events;

    char msg[1024];
    int ret = read(fd, msg, sizeof(msg));
    if(ret < 0) {
        std::cerr << "read faild." << std::endl;
        exit(-1);
    }

    struct bufferevent *bev = (struct bufferevent *)arg;

    bufferevent_write(bev, msg, ret);
}

void server_msg_cb(struct bufferevent *bev, void *arg) {
    (void)arg;

    char msg[1024];
    size_t len = bufferevent_read(bev, msg, sizeof(msg));
    msg[len] = '\0';

    std::cout << "message that received from server: " << msg << std::endl;
}

void event_cb(struct bufferevent *bev, short events, void *arg) {
    if(events & BEV_EVENT_EOF) {
        std::cerr << "connection closed." << std::endl;
    } else if( events & BEV_EVENT_ERROR ) {
        std::cerr << "some other error." << std::endl;
    } else {

    }

    bufferevent_free(bev);

    struct event *ev = (struct event *)arg;
    event_free(ev);
}

int tcp_connect_server(const char *server_ip, int port) {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    int status = inet_aton(server_ip, &addr.sin_addr);
    if(status == 0) { // invaild value.
        errno = EINVAL;
        return -1;
    }

    int sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1) {
        return sockfd;
    }

    status = connect(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    if(status == -1) {
        int save_errno = errno;
        ::close(sockfd);
        errno = save_errno;
        return -1;
    }

    evutil_make_socket_nonblocking(sockfd);

    return sockfd;
}
