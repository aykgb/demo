#include <iostream>
#include <string>

using namespace std;

extern "C" {
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
}

#define BUFFER_SIZE 1024

// char *socket_path = "./socket";
std::string socket_path = "\0hidden";
// std::string socket_path = "tst_unix_domain_socket";

static inline void print_error_and_exit(std::string info) {
    std::cout << __func__ << ":" <<  __LINE__ << " "
       <<  info << std::endl;
    exit(EXIT_FAILURE);
}

int main()
{
    /* 1. unlink path. */
    unlink(socket_path.c_str());

    /* 2. create listen addr. */
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    // addr.sun_path = socket_path.c_str(); // Path is acting like port.
    if(socket_path[0] == '\0') {
        // We need escape the first '\0' character which is for hidden use.
        *addr.sun_path = '\0';
        strncpy(addr.sun_path + 1, socket_path.c_str() + 1, sizeof(addr.sun_path) - 2);
    } else {
        strncpy(addr.sun_path, socket_path.c_str(), sizeof(addr.sun_path) - 1);
    }

    /* 3. create listen socket fd. */
    int lfd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if( lfd == -1 ) {
        print_error_and_exit("listen fd creation is failed.");
    }

    /* 4. bind. */
    int ret = bind(lfd, (const struct sockaddr*)&addr, sizeof(struct sockaddr_un));
    if(ret == -1) {
        print_error_and_exit("bind produrce is failed.");
    }

    /* 5. listen. */
    ret = listen(lfd, 20);
    if(ret == -1) {
        print_error_and_exit("listen produrce is failed.");
    }

    char buf[BUFFER_SIZE];
    while(true) {
        /* 6. accept a client connection and return the connected fd. */
        int incomming_fd = accept(lfd, NULL, NULL);
        if(incomming_fd == -1) {
            std::cout << "accept error.\n";
            continue;
        }

        while(true) {
            /* 7. read data from the connected fd. */
            // read is a sync interface and it returns the bytes read from the socket fd.
            ret = read(incomming_fd, buf, BUFFER_SIZE);

            if(ret == -1) {
                print_error_and_exit("read produrce is failed.");
            } else if (ret == 0) {
                // complete data receiving.
                printf("EOF\n");
                close(incomming_fd);
            } else {
                buf[BUFFER_SIZE - 1] = '\0';
                printf("read %u bytes: %.*s\n", ret, ret, buf);
            }
        }
    }

    return 0;
}
