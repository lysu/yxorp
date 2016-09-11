#include <stdlib.h>

#include "yxorp_core.h"

extern struct settings settings;
struct thread_worker *threads;
static int last_thread;

static int init_count;
static pthread_mutex_t init_lock;
static pthread_cond_t init_cond;

static rstatus_t thread_setup(struct thread_worker *t) {
    t->base = aeCreateEventLoop(10);
    if (t->base == NULL) {
        log_error("event init failed: %s", strerror(errno));
        return YX_ERROR;
    }

    return YX_OK;
}

static void *thread_worker_main(void *arg) {
    struct thread_worker *t = arg;

    pthread_mutex_lock(&init_lock);
    t->tid = pthread_self();
    init_count++;
    pthread_cond_signal(&init_cond);
    pthread_mutex_unlock(&init_lock);

    printf("work running\n");

    aeMain(t->base);

    return NULL;
}

static rstatus_t thread_create(thread_func_t func, void *arg) {
    pthread_t thread;
    err_t err;

    err = pthread_create(&thread, NULL, func, arg);
    if (err != 0) {
        log_error("pthread create failed: %s", strerror(err));
        return YX_ERROR;
    }

    return YX_OK;
}

rstatus_t thread_init(aeEventLoop *main_loop) {
    rstatus_t status;
    err_t err;
    int nworkers = settings.num_workers;
    struct thread_worker *dispatcher;
    int i;

    init_count = 0;
    pthread_mutex_init(&init_lock, NULL);
    pthread_cond_init(&init_cond, NULL);

    last_thread = -1;

    /* dispatcher takes the extra (last) slice of thread descriptor */
    threads = zmalloc(sizeof(*threads) * (1 + nworkers));
    if (threads == NULL) {
        return YX_ENOMEM;
    }
    /* keep data of dispatcher close to worker threads for easy aggregation */
    dispatcher = &threads[nworkers];

    dispatcher->base = main_loop;
    dispatcher->tid = pthread_self();

    for (i = 0; i < nworkers; i++) {
        status = thread_setup(&threads[i]);
        if (status != YX_OK) {
            return status;
        }
    }

    for (i = 0; i < nworkers; i++) {
        status = thread_create(thread_worker_main, &threads[i]);
        if (status != YX_OK) {
            return status;
        }
    }

    /* wait for all the workers to set themselves up */
    pthread_mutex_lock(&init_lock);
    while (init_count < nworkers) {
        pthread_cond_wait(&init_cond, &init_lock);
    }
    pthread_mutex_unlock(&init_lock);

    /* wait for all the workers and dispatcher to set themselves up */
    pthread_mutex_lock(&init_lock);
    while (init_count < nworkers) { /* +2: aggregator & klogger */
        pthread_cond_wait(&init_cond, &init_lock);
    }
    pthread_mutex_unlock(&init_lock);

    return YX_OK;
}

void thread_deinit(void) {}
