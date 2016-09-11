#ifndef MAIN_H
#define MAIN_H

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <unistd.h>

#include "yxorp_net.h"
#include "../lib/zmalloc/includes/zmalloc.h"

#define LISTENQ 5

typedef struct config {
    int threads;
    int port;
} config;

static int parse_args(config *cfg, int argc, char **argv);
static void *thread_main(void *);

static int socket_bind(int port);
static int set_listen_flag(int fd);

static int accept_loop(int listenfd, thread *threads, int threads_cnt);
static void socket_accepted(aeEventLoop *loop, int fd, void *data, int mask);

#endif
