cmake_minimum_required(VERSION 3.10)
project(Moxi C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Build options
option(BUILD_TESTS "Build test executables" OFF)
option(BUILD_TOOLS "Build vbucket tools" ON)

# Find required packages
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

# Find libevent
pkg_check_modules(LIBEVENT REQUIRED libevent)

# Find libcurl  
pkg_check_modules(CURL REQUIRED libcurl)

# Find OpenSSL
find_package(OpenSSL REQUIRED)

# Find zlib
find_package(ZLIB REQUIRED)

# Find cJSON - try pkg-config first, then manual search
pkg_check_modules(CJSON libcjson)
if(NOT CJSON_FOUND)
    # Try to find cJSON manually
    find_path(CJSON_INCLUDE_DIRS cJSON.h 
              PATHS /usr/include /usr/local/include)
    find_library(CJSON_LIBRARIES NAMES cjson cJSON
                 PATHS /usr/lib /usr/local/lib /usr/lib64 /usr/local/lib64)
    if(CJSON_INCLUDE_DIRS AND CJSON_LIBRARIES)
        set(CJSON_FOUND TRUE)
        message(STATUS "Found cJSON: ${CJSON_LIBRARIES}")
    else()
        message(FATAL_ERROR "cJSON library not found. Install cjson-devel or libcjson-dev")
    endif()
endif()

# Compiler flags
add_compile_options(-Wall -Wextra -Wno-unused-parameter -Wno-sign-compare)
add_compile_options(-fno-strict-aliasing)

# For older code that uses some deprecated features
add_compile_options(-Wno-deprecated-declarations)

# Performance optimization flags for Release builds
if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    add_compile_options(-O3 -march=native -mtune=native)
    add_compile_options(-flto)  # Link-time optimization
    add_compile_options(-funroll-loops)
    add_link_options(-flto)
endif()

# Define for standalone build
add_definitions(-DMOXI_ITEM_MALLOC=1)
add_definitions(-DSTANDALONE_BUILD=1)

# Include directories - our platform shim comes first
include_directories(BEFORE 
    ${CMAKE_CURRENT_SOURCE_DIR}/platform
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/libmemcached
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/src
)

include_directories(
    ${LIBEVENT_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIRS}
    ${CJSON_INCLUDE_DIRS}
)

link_directories(
    ${LIBEVENT_LIBRARY_DIRS}
    ${CURL_LIBRARY_DIRS}
    ${CJSON_LIBRARY_DIRS}
)

# Check for required functions
include(CheckFunctionExists)
include(CheckIncludeFiles)
include(CheckSymbolExists)

check_include_files("umem.h" HAVE_UMEM_H)
check_include_files("sysexits.h" HAVE_SYSEXITS_H)
check_function_exists(getpwnam HAVE_GETPWNAM)
check_function_exists(getrlimit HAVE_GETRLIMIT)
check_function_exists(mlockall HAVE_MLOCKALL)
check_function_exists(getpagesizes HAVE_GETPAGESIZES)

# Version info
set(PRODUCT_VERSION "6.0.0")
set(CONFLATE_DB_PATH "${CMAKE_INSTALL_PREFIX}/var/lib/moxi")

# Configure header
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config.cmake.h
    ${CMAKE_CURRENT_BINARY_DIR}/src/config.h
)

# ============================================================================
# libhashkit (from bundled libmemcached)
# ============================================================================
add_library(hashkit SHARED
    libmemcached/libhashkit/algorithm.c
    libmemcached/libhashkit/behavior.c
    libmemcached/libhashkit/crc32.c
    libmemcached/libhashkit/fnv.c
    libmemcached/libhashkit/digest.c
    libmemcached/libhashkit/function.c
    libmemcached/libhashkit/hashkit.c
    libmemcached/libhashkit/jenkins.c
    libmemcached/libhashkit/ketama.c
    libmemcached/libhashkit/md5.c
    libmemcached/libhashkit/one_at_a_time.c
    libmemcached/libhashkit/strerror.c
)
target_compile_definitions(hashkit PRIVATE BUILDING_HASHKIT=1)
set_target_properties(hashkit PROPERTIES SOVERSION 1.0.0)
target_link_libraries(hashkit m)

# Configure libmemcached
check_symbol_exists(htonll arpa/inet.h HAVE_HTONLL)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/libmemcached/libmemcached_config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/libmemcached_config.h
)

# ============================================================================
# libmcd (from bundled libmemcached)
# ============================================================================
add_library(mcd SHARED
    libmemcached/libmemcached/allocators.c
    libmemcached/libmemcached/analyze.c
    libmemcached/libmemcached/auto.c
    libmemcached/libmemcached/behavior.c
    libmemcached/libmemcached/byteorder.c
    libmemcached/libmemcached/connect.c
    libmemcached/libmemcached/delete.c
    libmemcached/libmemcached/do.c
    libmemcached/libmemcached/dump.c
    libmemcached/libmemcached/fetch.c
    libmemcached/libmemcached/flush.c
    libmemcached/libmemcached/flush_buffers.c
    libmemcached/libmemcached/get.c
    libmemcached/libmemcached/hash.c
    libmemcached/libmemcached/hosts.c
    libmemcached/libmemcached/io.c
    libmemcached/libmemcached/key.c
    libmemcached/libmemcached/memcached.c
    libmemcached/libmemcached/parse.c
    libmemcached/libmemcached/purge.c
    libmemcached/libmemcached/quit.c
    libmemcached/libmemcached/response.c
    libmemcached/libmemcached/result.c
    libmemcached/libmemcached/server.c
    libmemcached/libmemcached/server_list.c
    libmemcached/libmemcached/stats.c
    libmemcached/libmemcached/string.c
    libmemcached/libmemcached/storage.c
    libmemcached/libmemcached/strerror.c
    libmemcached/libmemcached/verbosity.c
    libmemcached/libmemcached/version.c
)
target_include_directories(mcd PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/libmemcached
    ${CMAKE_CURRENT_BINARY_DIR}
)
set_target_properties(mcd PROPERTIES SOVERSION 6.1.0)
target_link_libraries(mcd hashkit)

# ============================================================================
# libconflate
# ============================================================================
add_library(conflate SHARED
    conflate/adhoc_commands.c
    conflate/conflate.c
    conflate/kvpair.c
    conflate/logging.c
    conflate/persist.c
    conflate/rest.c
    conflate/util.c
    conflate/xmpp.c
)
target_compile_definitions(conflate PRIVATE BUILDING_LIBCONFLATE=1)
set_target_properties(conflate PROPERTIES SOVERSION 1.0.0)
target_link_libraries(conflate 
    ${CURL_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    Threads::Threads
)

# ============================================================================
# libvbucket
# ============================================================================
add_library(vbucket SHARED
    vbucket/crc32.c
    vbucket/ketama.c
    vbucket/vbucket.c
)
target_compile_definitions(vbucket PRIVATE BUILDING_LIBVBUCKET=1)
set_target_properties(vbucket PROPERTIES SOVERSION 1.1.1)
target_link_libraries(vbucket 
    ${CJSON_LIBRARIES}
    m
    Threads::Threads
)

# ============================================================================
# moxi executable
# ============================================================================
add_executable(moxi
    src/memcached.c
    src/genhash.c
    src/hash.c
    src/slabs.c
    src/items.c
    src/assoc.c
    src/thread.c
    src/stats.c
    src/util.c
    src/work.c
    src/cproxy.c
    src/cproxy_config.c
    src/cproxy_protocol_a.c
    src/cproxy_protocol_a2a.c
    src/cproxy_protocol_a2b.c
    src/cproxy_protocol_b.c
    src/cproxy_protocol_b2b.c
    src/cproxy_multiget.c
    src/cproxy_stats.c
    src/cproxy_front.c
    src/matcher.c
    src/murmur_hash.c
    src/mcs.c
    src/stdin_check.c
    src/log.c
    src/htgram.c
    src/agent_config.c
    src/agent_ping.c
    src/agent_stats.c
    src/daemon.c
    src/cache.c
    src/strsep.c
)

target_link_libraries(moxi
    conflate
    vbucket
    mcd
    ${LIBEVENT_LIBRARIES}
    Threads::Threads
)

# ============================================================================
# Tools
# ============================================================================
if(BUILD_TOOLS)
    add_executable(vbuckettool vbucket/vbuckettool.c)
    target_link_libraries(vbuckettool vbucket)
    
    add_executable(vbucketkeygen vbucket/vbucketkeygen.c)
    target_link_libraries(vbucketkeygen vbucket)
    
    install(TARGETS vbuckettool vbucketkeygen
            RUNTIME DESTINATION bin)
endif()

# ============================================================================
# Tests
# ============================================================================
if(BUILD_TESTS)
    enable_testing()
    
    add_executable(moxi_sizes tests/moxi/sizes.c)
    
    add_executable(moxi_htgram_test tests/moxi/htgram_test.c src/htgram.c)
    target_link_libraries(moxi_htgram_test Threads::Threads)
    
    add_executable(vbucket_testapp tests/vbucket/testapp.c)
    target_link_libraries(vbucket_testapp vbucket)
    
    add_executable(vbucket_regression tests/vbucket/regression.c)
    target_link_libraries(vbucket_regression vbucket)
    
    add_executable(vbucket_testketama 
        vbucket/ketama.c
        tests/vbucket/testketama.c
    )
    target_link_libraries(vbucket_testketama vbucket)
    
    add_executable(tests_check_kvpair
        tests/conflate/check_kvpair.c
        tests/conflate/test_common.c
    )
    target_link_libraries(tests_check_kvpair conflate)
    
    add_test(NAME moxi-sizes COMMAND moxi_sizes)
    add_test(NAME moxi-htgram-test COMMAND moxi_htgram_test)
    add_test(NAME vbucket-basic-tests COMMAND vbucket_testapp ${CMAKE_CURRENT_SOURCE_DIR})
    add_test(NAME vbucket-regression-tests COMMAND vbucket_regression ${CMAKE_CURRENT_SOURCE_DIR})
    add_test(NAME vbucket-ketama-tests COMMAND vbucket_testketama)
    add_test(NAME libconflate-test-suite COMMAND tests_check_kvpair)
endif()

# ============================================================================
# Installation
# ============================================================================
install(TARGETS moxi
        RUNTIME DESTINATION bin)

install(TARGETS conflate vbucket mcd hashkit
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

# Create var directory for conflate db
install(DIRECTORY DESTINATION var/lib/moxi)

# ============================================================================
# Package info
# ============================================================================
message(STATUS "")
message(STATUS "Moxi ${PRODUCT_VERSION} configuration:")
message(STATUS "  libevent: ${LIBEVENT_LIBRARIES}")
message(STATUS "  libcurl:  ${CURL_LIBRARIES}")  
message(STATUS "  cJSON:    ${CJSON_LIBRARIES}")
message(STATUS "  OpenSSL:  ${OPENSSL_LIBRARIES}")
message(STATUS "  zlib:     ${ZLIB_LIBRARIES}")
message(STATUS "")
