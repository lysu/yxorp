#include <netdb.h>
#include <stdio.h>
#include <sys/file.h>

#include "yxorp_core.h"

extern struct settings settings;
static aeEventLoop *main_loop;
static pthread_mutex_t accept_lock;

rstatus_t core_init(void) {
    rstatus_t status;

    status = log_init(settings.verbose, settings.log_filename);
    if (status != YX_OK) {
        return status;
    }

    status = signal_init();
    if (status != YX_OK) {
        return status;
    }

    pthread_mutex_init(&accept_lock, NULL);

    main_loop = aeCreateEventLoop(10);
    if (main_loop == NULL) {
        return YX_ERROR;
    }

    status = thread_init(main_loop);
    if (status != YX_OK) {
        return status;
    }

    return YX_OK;
}

static void socket_accepted(aeEventLoop *loop, int fd, void *data, int mask) {
    int clifd;
    struct sockaddr_in cliaddr;
    socklen_t cliaddrlen;
    clifd = accept(fd, (struct sockaddr *)&cliaddr, &cliaddrlen);
    printf("accept %d\n", clifd);
}

static rstatus_t core_create_socket(void) {
    rstatus_t status;
    int sd;
    struct addrinfo *ai;
    struct addrinfo *next;
    struct addrinfo hints = {.ai_flags = AI_PASSIVE, .ai_family = AF_UNSPEC};
    char port_buf[NI_MAXSERV];
    int error;
    int success = 0;
    hints.ai_socktype = SOCK_STREAM;
    snprintf(port_buf, sizeof(port_buf), "%d", settings.port);
    error = getaddrinfo(settings.interface, port_buf, &hints, &ai);
    if (error != 0) {
        log_error("getaddrinfo() failed: %s", gai_strerror(error));
        return YX_ERROR;
    }

    for (next = ai; next != NULL; next = next->ai_next) {
        sd = socket(next->ai_family, next->ai_socktype, next->ai_protocol);
        if (sd < 0) {
            continue;
        }

        status = yx_set_nonblocking(sd);
        if (status != YX_OK) {
            log_error("set nonblock on sd %d failed: %s", sd, strerror(errno));
            close(sd);
            continue;
        }

        status = yx_set_reuseaddr(sd);
        if (status != YX_OK) {
            log_warn("set reuse addr on sd %d failed, ignored: %s", sd,
                     strerror(errno));
        }

        status = bind(sd, next->ai_addr, next->ai_addrlen);
        if (status != YX_OK) {
            if (errno != EADDRINUSE) {
                log_error("bind on sd %d failed: %s", sd, strerror(errno));
                close(sd);
                freeaddrinfo(ai);
                return YX_ERROR;
            }
            close(sd);
            continue;
        }

        success++;

        if (listen(sd, settings.backlog) == -1) {
            log_error("listen on sd %d failed: %s", sd, strerror(errno));
            close(sd);
            freeaddrinfo(ai);
            return YX_ERROR;
        }

        printf("start listening\n");

        if (aeCreateFileEvent(main_loop, sd, AE_READABLE, socket_accepted,
                              NULL) != AE_OK) {
            return YX_ERROR;
        }

        log_debug(LOG_NOTICE, "s %d listening", conn->sd);
    }

    freeaddrinfo(ai);

    return success != 0 ? YX_OK : YX_ERROR;
}

rstatus_t core_loop(void) {
    rstatus_t status;

    status = core_create_socket();
    if (status != YX_OK) {
        return status;
    }

    aeMain(main_loop);

    return YX_OK;
}
