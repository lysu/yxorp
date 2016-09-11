#ifndef _YX_THREAD_H_
#define _YX_THREAD_H_

#include <pthread.h>
#include <semaphore.h>
#include "ae.h"

typedef void *(*thread_func_t)(void *);

struct thread_worker {
    pthread_t tid;
    aeEventLoop *base;
};

int thread_init(aeEventLoop *main_loop);
void thread_deinit(void);

#endif
