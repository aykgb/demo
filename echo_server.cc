#include <iostream>
#include "config.hpp"

int main()
{
    int ret = 0;
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET; // using IPv4 address;
    server_addr.sin_addr.s_addr = inet_addr(IPADDR);
    server_addr.sin_port = htons(PORT);

    int server_socket_fd =
        socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // create socket fd;
    if(server_socket_fd == -1) {
        handle_error("socket");
    }

    ret = bind(server_socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(ret == -1) {
        handle_error("bind");
    }
    ret = listen(server_socket_fd, 20);
    if( ret == -1 ) {
        handle_error("listen");
    }

    struct sockaddr_in client_addr;
    socklen_t length_of_client_addr = sizeof(client_addr);

    int client_socket_fd =
        accept(server_socket_fd, (struct sockaddr*)&client_addr, &length_of_client_addr);
    if(client_socket_fd == -1) {
        handle_error("accept");
    }

    char msg[] = "Hello socket world!";
    write(client_socket_fd, msg, sizeof(msg));

    close(client_socket_fd);
    close(server_socket_fd);

    return 0;
}
