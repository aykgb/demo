#include <iostream>
#include "config.hpp"

int main()
{
    int ret = 0;
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(IPADDR);
    server_addr.sin_port = htons(PORT);

    int socket_fd =
        socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(socket_fd == -1) {
        handle_error("socket");
    }

    ret = connect(socket_fd, (struct sockaddr*)& server_addr, sizeof(server_addr));
    if(ret == -1) {
        handle_error("connect");
    }

    char buf[1024];
    memset(buf, 0, 1024);
    read(socket_fd, buf, sizeof(buf) - 1);

    std::cout << "Message from server:\n    "
              <<  buf << std::endl;

    close(socket_fd);


    return 0;
}
