#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <assert.h>

#define IPADDR  "127.0.0.1"
#define PORT    8787
#define MAXLINE 1024
#define LISTENQ 5
#define SIZE    10

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while(0)

