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

    return 0;
}


