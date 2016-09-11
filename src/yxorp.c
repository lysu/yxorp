#include <sys/resource.h>

#include "yxorp_core.h"

struct settings settings;

static void yx_set_default_options(void) {
    memset(&settings, 0, sizeof(struct settings));
    settings.num_workers = 2;
    settings.port = 9001;
    settings.verbose = LOG_NOTICE;
    settings.maxconns = 1024;
    settings.backlog = 1024;
    settings.interface = NULL;
}

static rstatus_t yx_get_options(int argc, char** argv) { return YX_OK; }

static void yx_show_usage(void) {
    printf(
        "Usage: yxorp <options>      \n"
        "Options:                    \n");
}

static rstatus_t yx_set_maxconns(void) {
    int status, maxfiles;
    struct rlimit rlim;

    status = getrlimit(RLIMIT_NOFILE, &rlim);
    if (status != 0) {
        log_stderr("twemcache: getrlimit(RLIMIT_NOFILE) failed: %s",
                   strerror(errno));
        return YX_ERROR;
    }

    maxfiles = settings.maxconns;

    rlim.rlim_cur = MAX(rlim.rlim_cur, maxfiles);
    rlim.rlim_max = MAX(rlim.rlim_max, rlim.rlim_cur);

    status = setrlimit(RLIMIT_NOFILE, &rlim);
    if (status != 0) {
        log_stderr("twemcache: setting open files limit to %d failed: %s",
                   maxfiles, strerror(errno));
        log_stderr(
            "twemcache: try running as root or request smaller "
            "--max-conns value");
        return YX_ERROR;
    }

    return YX_OK;
}

int main(int argc, char** argv) {
    rstatus_t status;

    yx_set_default_options();

    status = yx_get_options(argc, argv);
    if (status != YX_OK) {
        yx_show_usage();
        exit(EXIT_FAILURE);
    }

    status = yx_set_maxconns();
    if (status != YX_OK) {
        exit(EXIT_FAILURE);
    }

    settings.pid = getpid();

    status = core_init();
    if (status != YX_OK) {
        exit(EXIT_FAILURE);
    }

    status = core_loop();
    if (status != YX_OK) {
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
