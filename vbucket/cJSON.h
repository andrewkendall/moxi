/* Wrapper to find system cJSON header */
#ifndef CJSON_WRAPPER_H
#define CJSON_WRAPPER_H

/* Try different common locations for cJSON */
#if __has_include(<cjson/cJSON.h>)
#include <cjson/cJSON.h>
#elif __has_include(<cJSON/cJSON.h>)
#include <cJSON/cJSON.h>
#elif __has_include(<cJSON.h>)
#include <cJSON.h>
#else
#error "cJSON.h not found - install cjson-devel or libcjson-dev"
#endif

#endif /* CJSON_WRAPPER_H */
