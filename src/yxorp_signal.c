#include <signal.h>
#include <stdlib.h>

#include "yxorp_core.h"

static struct signal signals[] = {
    {SIGUSR1, "SIGUSR1", 0, signal_handler},
    {SIGUSR2, "SIGUSR2", 0, signal_handler},
    {SIGTTIN, "SIGTTIN", 0, signal_handler},
    {SIGTTOU, "SIGTTOU", 0, signal_handler},
    {SIGHUP, "SIGHUP", 0, signal_handler},
    {SIGINT, "SIGINT", 0, signal_handler},
    {SIGTERM, "SIGTERM", 0, signal_handler},
    {SIGSEGV, "SIGSEGV", SA_RESETHAND, signal_handler},
    {SIGPIPE, "SIGPIPE", 0, SIG_IGN},
    {0, NULL, 0, NULL}};

#ifdef HAVE_GCOV
void __gcov_flush(void);
#endif

rstatus_t signal_init(void) {
    struct signal *sig;

    for (sig = signals; sig->signo != 0; sig++) {
        rstatus_t status;
        struct sigaction sa;

        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = sig->handler;
        sa.sa_flags = sig->flags;
        sigemptyset(&sa.sa_mask);

        status = sigaction(sig->signo, &sa, NULL);
        if (status < 0) {
            log_error("sigaction(%s) failed: %s", sig->signame,
                      strerror(errno));
            return YX_ERROR;
        }
    }

    return YX_OK;
}

void signal_deinit(void) {}

#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 6
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif
void signal_handler(int signo) {
    struct signal *sig;
    void (*action)(void);
    char *actionstr;
    bool done;

    for (sig = signals; sig->signo != 0; sig++) {
        if (sig->signo == signo) {
            break;
        }
    }
    ASSERT(sig->signo != 0);

    actionstr = "";
    action = NULL;
    done = false;

    switch (signo) {
        case SIGUSR1:
            break;

        case SIGUSR2:
            break;

        case SIGTTIN:
            actionstr = ", up logging level";
            action = log_level_up;
            break;

        case SIGTTOU:
            actionstr = ", down logging level";
            action = log_level_down;
            break;

        case SIGHUP:
            actionstr = ", reopening log file";
            action = log_reopen;
            break;

        case SIGINT:
        case SIGTERM:
            done = true;
            actionstr = ", exiting";
            break;

        case SIGSEGV:
            yx_stacktrace(1);
            actionstr = ", core dumping";
            raise(SIGSEGV);
            break;

        default:
            NOT_REACHED();
    }

    log_debug(LOG_NOTICE, "signal %d (%s) received%s", signo, sig->signame,
              actionstr);

    if (action != NULL) {
        action();
    }

    if (done) {
#ifdef HAVE_GCOV
        __gcov_flush();
#endif
        exit(1);
    }
}
#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 6
#pragma GCC diagnostic pop
#endif
