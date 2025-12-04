# Moxi 6.0.0 - Standalone Build for Modern Systems

This is a modified version of Couchbase's moxi proxy that builds standalone
on modern Linux and macOS systems without requiring the full Couchbase build
infrastructure.

## What is Moxi?

Moxi is a memcached proxy that provides:
- Connection pooling to downstream memcached servers
- Consistent hashing (ketama) for distributing keys
- vBucket-aware routing for Couchbase clusters
- ASCII and binary protocol support
- Simple static configuration or REST-based dynamic configuration

## Building

### Prerequisites

Install the required development packages:

**Rocky Linux 8/9 / RHEL 8/9:**
```bash
sudo dnf install -y epel-release
sudo dnf install -y cmake gcc make \
    libevent-devel libcurl-devel openssl-devel \
    zlib-devel cjson-devel
```

**Ubuntu / Debian:**
```bash
sudo apt-get update
sudo apt-get install -y cmake gcc make \
    libevent-dev libcurl4-openssl-dev libssl-dev \
    zlib1g-dev libcjson-dev
```

**Fedora:**
```bash
sudo dnf install -y cmake gcc make \
    libevent-devel libcurl-devel openssl-devel \
    zlib-devel cjson-devel
```

### Quick Build

```bash
./build.sh release
```

This will create binaries in the `build/` directory.

### Manual Build

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

### Installation

```bash
cd build
sudo make install
```

This installs to `/usr/local` by default. To change:

```bash
cmake -DCMAKE_INSTALL_PREFIX=/opt/moxi ..
```

## Usage

### Simple Proxy Mode

Proxy port 11211 to a single memcached server:
```bash
./moxi -z 11211=localhost:11212
```

Proxy to multiple servers (consistent hashing):
```bash
./moxi -z 11211=server1:11211,server2:11211,server3:11211
```

### Configuration File Mode

Create a vbucket configuration file (`config.cfg`):
```
11211 = {
    "hashAlgorithm": "CRC",
    "numReplicas": 0,
    "serverList": ["memcached1:11211", "memcached2:11211"],
    "vBucketMap": [[0], [1]]
}
```

Start with config file:
```bash
./moxi -z ./config.cfg
```

### Common Options

```
-p <port>       TCP port to listen on (default: 11211)
-U <port>       UDP port to listen on (default: 0, off)
-l <addr>       Interface to listen on (default: INADDR_ANY)
-c <num>        Max simultaneous connections (default: 1024)
-v              Verbose (multiple -v increase verbosity)
-d              Run as daemon
-u <username>   Run as user
-z <config>     Proxy configuration
```

## systemd Service

A systemd service file is provided in `contrib/moxi.service`.

```bash
# Copy service file
sudo cp contrib/moxi.service /etc/systemd/system/

# Edit configuration as needed
sudo systemctl edit moxi

# Enable and start
sudo systemctl enable moxi
sudo systemctl start moxi
```

## Building RPM

```bash
# Install rpm build tools
sudo dnf install -y rpm-build rpmdevtools

# Setup rpmbuild directory
rpmdev-setuptree

# Create tarball
tar czf ~/rpmbuild/SOURCES/moxi-6.0.0.tar.gz moxi-6.0.0/

# Build RPM
rpmbuild -ba contrib/moxi.spec
```

## Changes from Original

This standalone build includes the following modifications:

1. **Platform Shim** (`platform/platform.h`, `platform/cbassert.h`)
   - Maps Couchbase platform API to POSIX primitives
   - Provides `cb_mutex_*`, `cb_thread_*`, `cb_cond_*` using pthreads
   - Defines `cb_assert()` using standard `assert()`

2. **Standalone CMakeLists.txt**
   - Finds system libraries via pkg-config
   - Builds all bundled dependencies (libconflate, libvbucket, libmemcached)
   - No longer requires Couchbase build infrastructure

3. **cJSON Wrapper** (`vbucket/cJSON.h`)
   - Handles different system cJSON header locations

4. **Build Scripts**
   - `build.sh` - Easy build script with dependency checking
   - `contrib/moxi.spec` - RPM spec file
   - `contrib/moxi.service` - systemd service file

## Bundled Libraries

Moxi bundles the following libraries (no external downloads needed):

- **libconflate** - Configuration management
- **libvbucket** - vBucket routing and hashing
- **libmemcached/libhashkit** - Memcached client and hashing

## License

BSD License - see LICENSE file.

## Original Project

- Repository: https://github.com/couchbase/moxi
- This project is no longer actively maintained by Couchbase
