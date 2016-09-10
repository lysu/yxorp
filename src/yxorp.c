#include "yxorp.h"
#include "main.h"

static config cfg;

static struct sock sock = {.connect = sock_connect,
                           .close = sock_close,
                           .read = sock_read,
                           .write = sock_write,
                           .readable = sock_readable};

static volatile sig_atomic_t stop = 0;

static void handler(int sig) { stop = 1; }

static void usage() {
    printf(
        "Usage: yxorp <options>      \n"
        "Options:                    \n");
}

int main(int argc, char **argv) {
    int listenfd;

    if (parse_args(&cfg, argc, argv)) {
        usage();
        exit(1);
    }

    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, SIG_IGN);

    listenfd = socket_bind(cfg.port);
    if (listenfd == -1) {
        fprintf(stderr, "socket bind to %d error", cfg.port);
        exit(2);
    }

    if (listen(listenfd, LISTENQ) == -1) {
        fprintf(stderr, "socket listen to %d error", cfg.port);
        exit(2);
    }

    if (set_listen_flag(listenfd) == -1) {
        fprintf(stderr, "socket set listen flag %d error", cfg.port);
        exit(2);
    }

    thread *threads = zcalloc(cfg.threads * sizeof(thread));
    for (int i = 0; i < cfg.threads; i++) {
        thread *t = &threads[i];

        if (pthread_create(&t->thread, NULL, &thread_main, t)) {
            char *msg = strerror(errno);
            fprintf(stderr, "unable to create thread %d: %s\n", i, msg);
            exit(2);
        }
    }

    struct sigaction sa = {
        .sa_handler = handler, .sa_flags = 0,
    };
    sigfillset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    printf("Running as %d workers\n", cfg.threads);

    if (accept_loop(listenfd, threads, cfg.threads) == -1) {
        fprintf(stderr, "accept error");
        exit(2);
    }

    for (int i = 0; i < cfg.threads; i++) {
        thread *t = &threads[i];
        pthread_join(t->thread, NULL);
    }

    printf("Finished \n");
}

static int parse_args(config *cfg, int argc, char **argv) {
    memset(cfg, 0, sizeof(struct config));
    cfg->threads = 2;
    cfg->port = 9001;
    return 0;
}

static int socket_bind(int port) {
    int listenfd;
    struct sockaddr_in servaddr;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        goto error;
    }
    int on = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
        goto error;
    }
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port);
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        goto error;
    }
    printf("Bind %d at %d success\n", listenfd, port);
    return listenfd;
error:
    close(listenfd);
    return -1;
}

static int set_listen_flag(int fd) {
    if (fd < 0) {
        goto error;
    }
    int opts;
    opts = fcntl(fd, F_GETFL);
    if (opts < 0) {
        goto error;
    }
    opts = opts | O_NONBLOCK;
    if (fcntl(fd, F_SETFL, opts)) {
        goto error;
    }
    struct linger li = {
        .l_onoff = 1, .l_linger = 0,
    };
    if (setsockopt(fd, SOL_SOCKET, SO_LINGER, &li, sizeof(li)) == -1) {
        goto error;
    }
    int on = 1;
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on)) == -1) {
        goto error;
    }
    return 0;
error:
    return -1;
}

static int accept_loop(int listenfd, thread *threads, int threads_cnt) {
    aeEventLoop *loop;
    loop = aeCreateEventLoop(10);
    if (aeCreateFileEvent(loop, listenfd, AE_READABLE, socket_accepted,
                          threads) != AE_OK) {
        goto error;
    }
    aeMain(loop);
    aeDeleteEventLoop(loop);
    return 0;
error:
    return -1;
}

static void socket_accepted(aeEventLoop *loop, int fd, void *data, int mask) {
    int clifd;
    struct sockaddr_in cliaddr;
    socklen_t cliaddrlen;
    clifd = accept(fd, (struct sockaddr *)&cliaddr, &cliaddrlen);
    printf("accept %d\n", clifd);
}

void *thread_main(void *arg) {
    thread *thread = arg;
    printf("test\n");
    return NULL;
}
