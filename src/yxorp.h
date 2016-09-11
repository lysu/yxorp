#ifndef YXORP_H
#define YXORP_H

#include <inttypes.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "../lib/ae/includes/ae.h"

#define RECVBUF 8192

typedef struct {
    pthread_t thread;
    uint64_t start;
} thread;

typedef struct {
    char *buffer;
    size_t length;
    char *cursor;
} buffer;

typedef struct connection {
    thread *thread;
    int fd;
    char buf[RECVBUF];
} connection;

#endif
