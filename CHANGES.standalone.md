# Moxi 6.0.0 - Changelog

## Version 6.0.0 (December 2025)

### Major Changes

- **Complete standalone build system** - Rewrote CMakeLists.txt to build independently without Couchbase infrastructure
- **POSIX platform shim** - Added platform abstraction layer for threading, mutexes, and assertions
- **Modern compiler support** - Updated for C11 standard and modern Linux/macOS toolchains
- **Improved error handling** - Cleaned up logging and assertion behavior in downstream connections
- **Bug fixes from upstream** - Incorporated fixes from v5.0.0 including:
  - MB-25403: Remove ep_version statistic
  - MB-24509: Prevent truncation of documents set with noreply
  - CBSE-3717: Various connection handling improvements

---

## Summary of Standalone Build Changes

This section documents modifications made to the original moxi source to enable
standalone building on modern systems without requiring Couchbase build infrastructure.

## New Files Added

### platform/platform.h
POSIX platform shim that implements Couchbase's platform API:
- `cb_mutex_t`, `cb_mutex_initialize()`, `cb_mutex_enter()`, `cb_mutex_exit()`, `cb_mutex_try_enter()`
- `cb_cond_t`, `cb_cond_initialize()`, `cb_cond_wait()`, `cb_cond_signal()`, `cb_cond_broadcast()`
- `cb_thread_t`, `cb_thread_self()`, `cb_create_thread()`, `cb_join_thread()`
- `cb_initialize_sockets()` (no-op on POSIX)
- Thread wrapper to handle moxi's `void(*)(void*)` function signature vs pthread's `void*(*)(void*)`

### platform/cbassert.h
Simple wrapper that defines `cb_assert()` using standard `assert()`.

### CMakeLists.txt (complete rewrite)
Standalone CMake build configuration that:
- Uses pkg-config to find system libraries (libevent, libcurl, cJSON)
- Builds all bundled libraries (hashkit, mcd, conflate, vbucket)
- Configures proper include paths for the platform shim
- Provides options for BUILD_TESTS and BUILD_TOOLS

### vbucket/cJSON.h
Wrapper header to locate system cJSON regardless of installation path:
- Tries `<cjson/cJSON.h>` (common on RHEL/Rocky)
- Tries `<cJSON/cJSON.h>` (some systems)
- Tries `<cJSON.h>` (fallback)

### build.sh
User-friendly build script that:
- Checks for required dependencies
- Provides helpful installation instructions for missing packages
- Supports debug/release builds
- Handles clean builds

### contrib/moxi.service
systemd service file for running moxi as a system service.

### contrib/moxi.spec
RPM spec file for building packages on Rocky Linux/RHEL.

### README.standalone.md
Documentation for the standalone build process.

## Modified Files

### src/config.cmake.h
- Moved `#include <inttypes.h>` before PRIu64/PRIu32 definitions
- Ensures proper format specifier macros on modern systems

### libmemcached/libhashkit/common.h
- Added `#include <platform/cbassert.h>` to provide `cb_assert()` macro

## Original Files Preserved

The following original files were renamed with `.orig` suffix:
- CMakeLists.txt.orig
- libmemcached/CMakeLists.txt.orig

## Dependencies

The build requires these system packages:

### Rocky Linux 8/9 / RHEL 8/9
```
libevent-devel libcurl-devel openssl-devel zlib-devel cjson-devel
```

### Ubuntu / Debian
```
libevent-dev libcurl4-openssl-dev libssl-dev zlib1g-dev libcjson-dev
```

## Known Limitations

1. Some advanced Couchbase-specific features may not work (SASL auth, etc.)
2. Windows builds are not supported

## Build Tested On

- Rocky Linux 8 (x86_64)
- Rocky Linux 9 (x86_64)
- Ubuntu 22.04 LTS (x86_64)
- macOS (arm64/x86_64)

- Rocky Linux 8 (x86_64)
- Rocky Linux 9 (x86_64)
- Ubuntu 22.04 LTS (x86_64)
