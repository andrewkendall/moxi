/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * Platform abstraction shim for standalone moxi build
 * Maps Couchbase platform API to POSIX primitives
 */

#ifndef PLATFORM_PLATFORM_H
#define PLATFORM_PLATFORM_H

#include <pthread.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Threading primitives
 */
typedef pthread_mutex_t cb_mutex_t;
typedef pthread_cond_t cb_cond_t;
typedef pthread_t cb_thread_t;

/*
 * Assertion macro
 */
#define cb_assert(expr) assert(expr)

/*
 * Mutex operations
 */
static inline void cb_mutex_initialize(cb_mutex_t *mutex) {
    pthread_mutex_init(mutex, NULL);
}

static inline void cb_mutex_destroy(cb_mutex_t *mutex) {
    pthread_mutex_destroy(mutex);
}

static inline void cb_mutex_enter(cb_mutex_t *mutex) {
    int rv = pthread_mutex_lock(mutex);
    if (rv != 0) {
        fprintf(stderr, "cb_mutex_enter failed: %s\n", strerror(rv));
        abort();
    }
}

static inline void cb_mutex_exit(cb_mutex_t *mutex) {
    int rv = pthread_mutex_unlock(mutex);
    if (rv != 0) {
        fprintf(stderr, "cb_mutex_exit failed: %s\n", strerror(rv));
        abort();
    }
}

static inline int cb_mutex_try_enter(cb_mutex_t *mutex) {
    return pthread_mutex_trylock(mutex);
}

/*
 * Condition variable operations
 */
static inline void cb_cond_initialize(cb_cond_t *cond) {
    pthread_cond_init(cond, NULL);
}

static inline void cb_cond_destroy(cb_cond_t *cond) {
    pthread_cond_destroy(cond);
}

static inline void cb_cond_wait(cb_cond_t *cond, cb_mutex_t *mutex) {
    pthread_cond_wait(cond, mutex);
}

static inline void cb_cond_signal(cb_cond_t *cond) {
    pthread_cond_signal(cond);
}

static inline void cb_cond_broadcast(cb_cond_t *cond) {
    pthread_cond_broadcast(cond);
}

static inline void cb_cond_timedwait(cb_cond_t *cond, cb_mutex_t *mutex, 
                                      unsigned int msec) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += msec / 1000;
    ts.tv_nsec += (msec % 1000) * 1000000;
    if (ts.tv_nsec >= 1000000000) {
        ts.tv_sec++;
        ts.tv_nsec -= 1000000000;
    }
    pthread_cond_timedwait(cond, mutex, &ts);
}

/*
 * Thread operations
 */
static inline cb_thread_t cb_thread_self(void) {
    return pthread_self();
}

/*
 * Thread wrapper structure to handle void(*)(void*) functions
 * pthread expects void*(*)(void*) but moxi uses void(*)(void*)
 */
struct cb_thread_wrapper_arg {
    void (*func)(void *);
    void *arg;
};

static inline void *cb_thread_wrapper(void *arg) {
    struct cb_thread_wrapper_arg *wrapper = (struct cb_thread_wrapper_arg *)arg;
    void (*func)(void *) = wrapper->func;
    void *real_arg = wrapper->arg;
    free(wrapper);
    func(real_arg);
    return NULL;
}

/*
 * cb_create_thread - create a new thread
 * @id: pointer to thread handle (output)
 * @func: thread entry function (void(*)(void*) signature - Couchbase style)
 * @arg: argument to pass to func
 * @detached: if non-zero, create detached thread
 * Returns 0 on success, error code on failure
 */
static inline int cb_create_thread(cb_thread_t *id,
                                   void (*func)(void *),
                                   void *arg,
                                   int detached) {
    int rv;
    pthread_attr_t attr;
    struct cb_thread_wrapper_arg *wrapper;
    
    wrapper = (struct cb_thread_wrapper_arg *)malloc(sizeof(*wrapper));
    if (wrapper == NULL) {
        return ENOMEM;
    }
    wrapper->func = func;
    wrapper->arg = arg;
    
    pthread_attr_init(&attr);
    if (detached) {
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    }
    
    rv = pthread_create(id, &attr, cb_thread_wrapper, wrapper);
    pthread_attr_destroy(&attr);
    
    if (rv != 0) {
        free(wrapper);
    }
    
    return rv;
}

static inline int cb_join_thread(cb_thread_t id) {
    return pthread_join(id, NULL);
}

/*
 * Socket initialization (no-op on POSIX systems, needed for Windows)
 */
static inline void cb_initialize_sockets(void) {
    /* No-op on Linux/POSIX - sockets just work */
}

/*
 * Time functions
 */
static inline uint64_t cb_get_monotonic_seconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec;
}

static inline uint64_t gethrtime(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_PLATFORM_H */

/* htonll/ntohll for 64-bit byte swapping - not available on all systems */
#ifndef htonll
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define htonll(x) __builtin_bswap64(x)
#define ntohll(x) __builtin_bswap64(x)
#else
#define htonll(x) (x)
#define ntohll(x) (x)
#endif
#endif

