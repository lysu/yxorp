/*
 * twemcache - Twitter memcached.
 * Copyright (c) 2012, Twitter, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * * Neither the name of the Twitter nor the names of its contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _YX_UTIL_H_
#define _YX_UTIL_H_

#include <stdarg.h>
#include <stdbool.h>
#include <sys/un.h>

#define LF (uint8_t)10
#define CR (uint8_t)13
#define CRLF "\x0d\x0a"
#define CRLF_LEN (uint32_t)(sizeof(CRLF) - 1)
#define VALUE "VALUE "
#define VALUE_LEN sizeof(VALUE) - 1

#define NELEMS(a) ((sizeof(a)) / sizeof((a)[0]))

#define KB (1024)
#define MB (1024 * KB)
#define GB (1024 * MB)

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

/*
 * Length of 1 byte, 2 bytes, 4 bytes, 8 bytes and largest integral
 * type (uintmax_t) in ascii, including the null terminator '\0'
 *
 * From stdint.h, we have:
 * # define UINT8_MAX	(255)
 * # define UINT16_MAX	(65535)
 * # define UINT32_MAX	(4294967295U)
 * # define UINT64_MAX	(__UINT64_C(18446744073709551615))
 */
#define YX_UINT8_MAXLEN (3 + 1)
#define YX_UINT16_MAXLEN (5 + 1)
#define YX_UINT32_MAXLEN (10 + 1)
#define YX_UINT64_MAXLEN (20 + 1)
#define YX_UINTMAX_MAXLEN YX_UINT64_MAXLEN

/*
 * Make data 'd' or pointer 'p', n-byte aligned, where n is a power of 2
 * of 2.
 */
#define YX_ALIGNMENT sizeof(unsigned long) /* platform word */
#define YX_ALIGN(d, n) ((size_t)(((d) + (n - 1)) & ~(n - 1)))
#define YX_ALIGN_PTR(p, n) \
    (void *)(((uintptr_t)(p) + ((uintptr_t)n - 1)) & ~((uintptr_t)n - 1))

/*
 * Memory allocation and free wrappers.
 *
 * These wrappers enables us to loosely detect double free, dangling
 * pointer access and zero-byte alloc.
 */
#define yx_alloc(_s) _yx_alloc((size_t)(_s), __FILE__, __LINE__)

#define yx_zalloc(_s) _yx_zalloc((size_t)(_s), __FILE__, __LINE__)

#define yx_calloc(_n, _s) \
    _yx_calloc((size_t)(_n), (size_t)(_s), __FILE__, __LINE__)

#define yx_realloc(_p, _s) _yx_realloc(_p, (size_t)(_s), __FILE__, __LINE__)

#define yx_free(_p)                       \
    do {                                  \
        _yx_free(_p, __FILE__, __LINE__); \
        (_p) = NULL;                      \
    } while (0)

void *_yx_alloc(size_t size, const char *name, int line);
void *_yx_zalloc(size_t size, const char *name, int line);
void *_yx_calloc(size_t nmemb, size_t size, const char *name, int line);
void *_yx_realloc(void *ptr, size_t size, const char *name, int line);
void _yx_free(void *ptr, const char *name, int line);

int yx_set_blocking(int sd);
int yx_set_nonblocking(int sd);
int yx_set_reuseaddr(int sd);
int yx_set_tcpnodelay(int sd);
int yx_set_keepalive(int sd);
int yx_set_linger(int sd, int timeout);
int yx_unset_linger(int sd);
int yx_set_sndbuf(int sd, int size);
int yx_set_rcvbuf(int sd, int size);
int yx_get_soerror(int sd);
int yx_get_sndbuf(int sd);
int yx_get_rcvbuf(int sd);
void yx_maximize_sndbuf(int sd);

void yx_set_interval(struct timeval *timer, long interval);

#define yx_atoi(_line, _n) _yx_atoi((uint8_t *)_line, (size_t)_n)

int _yx_atoi(uint8_t *line, size_t n);
bool yx_valid_port(int n);

/*
 * Wrappers to read or write data to/from (multiple) buffers
 * to a file or socket descriptor.
 */
#define yx_read(_d, _b, _n) read(_d, _b, (size_t)(_n))

#define yx_readv(_d, _b, _n) readv(_d, _b, (int)(_n))

#define yx_write(_d, _b, _n) write(_d, _b, (size_t)(_n))

#define yx_writev(_d, _b, _n) writev(_d, _b, (int)(_n))

/*
 * Wrappers around strtoull, strtoll, strtoul, strtol that are safer and
 * easier to use. Returns true if conversion succeeds.
 */
bool yx_strtoull_len(const char *str, uint64_t *out, size_t len);
bool yx_strtoull(const char *str, uint64_t *out);
bool yx_strtoll(const char *str, int64_t *out);
bool yx_strtoul(const char *str, uint32_t *out);
bool yx_strtol(const char *str, int32_t *out);
bool yx_str2oct(const char *str, int32_t *out);

/*
 * Wrappers for defining custom assert based on whether macro
 * YX_ASSERT_PANIC or YX_ASSERT_LOG was defined at the moment
 * ASSERT was called.
 */
#if defined YX_ASSERT_PANIC && YX_ASSERT_PANIC == 1

#define ASSERT(_x)                                 \
    do {                                           \
        if (!(_x)) {                               \
            yx_assert(#_x, __FILE__, __LINE__, 1); \
        }                                          \
    } while (0)

#define NOT_REACHED() ASSERT(0)

#elif defined YX_ASSERT_LOG && YX_ASSERT_LOG == 1

#define ASSERT(_x)                                 \
    do {                                           \
        if (!(_x)) {                               \
            yx_assert(#_x, __FILE__, __LINE__, 0); \
        }                                          \
    } while (0)

#define NOT_REACHED() ASSERT(0)

#else

#define ASSERT(_x)

#define NOT_REACHED()

#endif

void yx_stacktrace(int skip_count);
void yx_stacktrace_fd(int fd);
void yx_assert(const char *cond, const char *file, int line, int panic);

#define yx_strlen(_s) strlen((const char *)(_s))
#define yx_strncmp(_s1, _s2, _n) \
    strncmp((const char *)(_s1), (const char *)(_s2), (size_t)(_n))
#define yx_strndup(_s, _n) (uint8_t *)strndup((const char *)(_s), (size_t)(_n));
#define yx_snprintf(_s, _n, ...) snprintf((char *)_s, _n, __VA_ARGS__)
#define yx_scnprintf(_s, _n, ...) _scnprintf((char *)_s, _n, __VA_ARGS__)
#define yx_vscnprintf(_s, _n, _f, _a) _vscnprintf((char *)_s, _n, _f, _a)

/**
  A (very) limited version of snprintf.
  @param   to   Destination buffer.
  @param   size    Size of destination buffer.
  @param   fmt  printf() style format string.
  @returns Number of bytes written, including terminating '\0'
  Supports 'd' 'i' 'u' 'x' 'p' 's' conversion.
  Supports 'l' and 'll' modifiers for integral types.
  Does not support any width/precision.
  Implemented with simplicity, and async-signal-safety in mind.
*/
int _safe_vsnprintf(char *to, size_t size, const char *fmt, va_list ap);
int _safe_snprintf(char *to, size_t size, const char *fmt, ...);

#define yx_safe_snprintf(_s, _n, ...) \
    _safe_snprintf((char *)(_s), (size_t)(_n), __VA_ARGS__)
#define yx_safe_vsnprintf(_s, _n, _f, _a) \
    _safe_vsnprintf((char *)(_s), (size_t)(_n), _f, _a)

int _scnprintf(char *buf, size_t size, const char *fmt, ...);
int _vscnprintf(char *buf, size_t size, const char *fmt, va_list args);

/*
 * Address resolution for internet (ipv4 and ipv6) and unix domain
 * socket address.
 */

struct sockinfo {
    int family;        /* socket address family */
    socklen_t addrlen; /* socket address length */
    union {
        struct sockaddr_in in;   /* ipv4 socket address */
        struct sockaddr_in6 in6; /* ipv6 socket address */
        struct sockaddr_un un;   /* unix domain address */
    } addr;
};

void yx_resolve_peer(int sd, char *buf, int size);

#endif
