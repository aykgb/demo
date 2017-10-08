#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <sys/epoll.h>

#define SERVER_PORT     (7778)
#define EPOLL_MAX_NUM   (2048)
#define BUFFER_MAX_LEN  (4096)

char buffer[BUFFER_MAX_LEN];

void str2upper(char *str) {
    for (int i = 0; i < strlen(str); i++) {
        str[i] = toupper(str[i]);
    }
}

int main(int argc, char **argv) {
    // server address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = (SERVER_PORT);

    // socket
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);

    // bind
    bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));

    // listen
    listen(listen_fd, 10);

    // epoll create
    int epfd = epoll_create(EPOLL_MAX_NUM);
    if(epfd < 0) {
        perror("epoll create.");
        goto END;
    }

    // listen_fd -> epoll
    struct epoll_event event, *my_events;
    event.events = EPOLLIN;
    event.data.fd = listen_fd;
    if(epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &event)){
        perror("epoll ctl add listen_fd");
        goto END;
    }
    my_events = (struct epoll_event* )malloc(sizeof(struct epoll_event) * EPOLL_MAX_NUM);

    while(1) {
        // epoll wait
        int active_fds_cnt = epoll_wait(epfd, my_events, EPOLL_MAX_NUM, -1);
        for(int i = 0; i < active_fds_cnt; i++) {
            // listen_fd
            // EPOLLIN
            // EPOLLOUT
        }
    }

END:
    close(epfd);
    close(listen_fd);
    return 0;
}
