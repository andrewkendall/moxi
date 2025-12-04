/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * Platform cbassert shim for standalone moxi build
 */

#ifndef PLATFORM_CBASSERT_H
#define PLATFORM_CBASSERT_H

#include <assert.h>

#define cb_assert(expr) assert(expr)

#endif /* PLATFORM_CBASSERT_H */
