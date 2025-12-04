#!/bin/bash
# Build script for moxi on Rocky Linux 8/9
# Usage: ./build.sh [clean|debug|release]

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_TYPE="${1:-release}"
BUILD_DIR="${SCRIPT_DIR}/build"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

echo_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

echo_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check for required packages
check_dependencies() {
    echo_info "Checking dependencies..."
    
    local missing=()
    
    # Check for cmake
    if ! command -v cmake &> /dev/null; then
        missing+=("cmake")
    fi
    
    # Check for gcc
    if ! command -v gcc &> /dev/null; then
        missing+=("gcc")
    fi
    
    # Check for pkg-config
    if ! command -v pkg-config &> /dev/null; then
        missing+=("pkgconfig")
    fi
    
    # Check for required libraries via pkg-config
    for lib in libevent libcurl; do
        if ! pkg-config --exists $lib 2>/dev/null; then
            case $lib in
                libevent) missing+=("libevent-devel") ;;
                libcurl) missing+=("libcurl-devel") ;;
            esac
        fi
    done
    
    # Check for cJSON - might be libcjson or cjson
    if ! pkg-config --exists libcjson 2>/dev/null; then
        # Check if header exists anyway
        if [ ! -f /usr/include/cjson/cJSON.h ] && [ ! -f /usr/include/cJSON.h ]; then
            missing+=("cjson-devel")
        fi
    fi
    
    # Check for zlib
    if [ ! -f /usr/include/zlib.h ]; then
        missing+=("zlib-devel")
    fi
    
    # Check for OpenSSL
    if [ ! -f /usr/include/openssl/ssl.h ]; then
        missing+=("openssl-devel")
    fi
    
    if [ ${#missing[@]} -gt 0 ]; then
        echo_error "Missing dependencies: ${missing[*]}"
        echo ""
        echo "On Rocky Linux 8/9, install with:"
        echo "  sudo dnf install -y epel-release"
        echo "  sudo dnf install -y ${missing[*]}"
        echo ""
        exit 1
    fi
    
    echo_info "All dependencies found."
}

# Clean build
clean_build() {
    echo_info "Cleaning build directory..."
    rm -rf "${BUILD_DIR}"
}

# Build moxi
build_moxi() {
    local cmake_build_type="Release"
    local cmake_flags=""
    
    case "$BUILD_TYPE" in
        debug)
            cmake_build_type="Debug"
            cmake_flags="-DBUILD_TESTS=ON"
            ;;
        release)
            cmake_build_type="Release"
            cmake_flags=""
            ;;
        clean)
            clean_build
            exit 0
            ;;
        *)
            echo_error "Unknown build type: $BUILD_TYPE"
            echo "Usage: $0 [clean|debug|release]"
            exit 1
            ;;
    esac
    
    check_dependencies
    
    echo_info "Creating build directory..."
    mkdir -p "${BUILD_DIR}"
    cd "${BUILD_DIR}"
    
    echo_info "Running CMake (${cmake_build_type})..."
    cmake -DCMAKE_BUILD_TYPE="${cmake_build_type}" \
          -DCMAKE_INSTALL_PREFIX=/usr/local \
          ${cmake_flags} \
          "${SCRIPT_DIR}"
    
    echo_info "Building..."
    make -j$(nproc)
    
    echo ""
    echo_info "Build complete!"
    echo ""
    echo "Binaries are in: ${BUILD_DIR}"
    echo "  - moxi:         ${BUILD_DIR}/moxi"
    echo "  - vbuckettool:  ${BUILD_DIR}/vbuckettool"
    echo "  - vbucketkeygen: ${BUILD_DIR}/vbucketkeygen"
    echo ""
    echo "To install system-wide:"
    echo "  cd ${BUILD_DIR} && sudo make install"
    echo ""
}

# Run tests
run_tests() {
    if [ ! -f "${BUILD_DIR}/Makefile" ]; then
        echo_error "Build directory not found. Run './build.sh debug' first."
        exit 1
    fi
    
    cd "${BUILD_DIR}"
    make test
}

# Main
case "$BUILD_TYPE" in
    test)
        run_tests
        ;;
    *)
        build_moxi
        ;;
esac
