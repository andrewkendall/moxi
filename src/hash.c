/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * Hash table
 *
 * Replaced with MurmurHash wrapper for performance.
 */
#include "memcached.h"

uint32_t murmur_hash(const char *key, size_t length);

uint32_t hash(const void *key, size_t length, const uint32_t initval) {
    (void)initval;
    return murmur_hash((const char *)key, length);
}
