#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_LINE    (1024)
#define SERVER_PORT (7778)

void setnoblocking(int fd) {
    int opts = 0;
    opts = fcntl(fd, F_GETFL);
    opts = opts | O_NONBLOCK;
    fcntl(fd, F_SETFL);
}

int main(int argc, char *argv[])
{
    (void)argv;
    int sockfd;
    char recvline[MAX_LINE + 1] = {0};

    struct sockaddr_in server_addr;

    if(argc != 2) {
        fprintf(stderr, "usage: ./client <SERVER_IP>\n");
        exit(0);
    }

    return 0;
}
