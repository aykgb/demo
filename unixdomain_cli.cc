#include <iostream>
#include <string>

using namespace std;

extern "C" {
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
}

#define BUFFER_SIZE 1024

// std::string socket_path = "tst_unix_domain_socket";
std::string socket_path = "\0hidden";

static inline void print_error_and_exit(std::string info) {
    std::cout << __func__ << ":" <<  __LINE__ << " "
       <<  info << std::endl;
    exit(EXIT_FAILURE);
}

int main(int argc, char * argv[])
{
    (void)argc;
    (void)argv;
    /* 1. create connect adddr. */
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    if(socket_path[0] == '\0') {
        *addr.sun_path =  '\0';
        strncpy(addr.sun_path + 1, socket_path.c_str() + 1, sizeof(addr.sun_path) - 2);
    } else {
        strncpy(addr.sun_path, socket_path.c_str(), sizeof(addr.sun_path) - 1);
    }

    /* 2. create socket fd of connection. */
    int cfd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if(cfd == -1) {
        print_error_and_exit("socket fd creation is failed.");
    }

    /* 3. connection the fd which is created above. */
    int ret = connect(cfd, (struct sockaddr*)&addr, sizeof(addr));
    if(ret == -1) {
        print_error_and_exit("connect fd failed.");
    }

    char buf[BUFFER_SIZE];
    while(true) {
        ret = read(STDIN_FILENO, buf, sizeof(buf));
        if(ret < 0) {
            print_error_and_exit("read from stdio error.");
            break;
        }

        int cnt = write(cfd, buf, ret);
        if(cnt != ret) {
            if(cnt > 0) {
                std::cout << "write partial.\n";
            } else {
                print_error_and_exit("write to socket failed.");
            }
        }
    }


    return 0;
}
