#ifndef MAIN_H
#define MAIN_H

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "ae.h"
#include "yxorp_log.h"
#include "yxorp_signal.h"
#include "yxorp_thread.h"
#include "yxorp_util.h"
#include "zmalloc.h"

#define LISTENQ 5

#define YX_OK 0
#define YX_ERROR -1
#define YX_EAGAIN -2
#define YX_ENOMEM -3

typedef int rstatus_t;
typedef int err_t;

struct settings {
    int num_workers;
    int port;

    char* log_filename;
    int verbose;

    int maxconns;
    int backlog;
    char* interface;
    pid_t pid;
};

rstatus_t core_init(void);
rstatus_t core_loop(void);

#endif
