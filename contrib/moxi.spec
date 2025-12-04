Name:           moxi
Version:        6.0.0
Release:        1%{?dist}
Summary:        Memcached proxy with vbucket support

License:        BSD
URL:            https://github.com/couchbase/moxi
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  cmake >= 3.10
BuildRequires:  gcc
BuildRequires:  make
BuildRequires:  libevent-devel
BuildRequires:  libcurl-devel
BuildRequires:  openssl-devel
BuildRequires:  zlib-devel
BuildRequires:  cjson-devel

Requires:       libevent
Requires:       libcurl
Requires:       openssl
Requires:       zlib
Requires:       cjson

%description
Moxi is a proxy that allows you to use memcached protocol to
communicate with a Couchbase cluster or standard memcached servers.
It provides features like connection pooling, consistent hashing,
and vbucket-aware request routing.

%prep
%setup -q

%build
mkdir -p build
cd build
%cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTS=OFF \
    -DBUILD_TOOLS=ON
%cmake_build

%install
cd build
%cmake_install

# Create directories
install -d %{buildroot}%{_localstatedir}/lib/moxi
install -d %{buildroot}%{_unitdir}

# Install systemd service
install -m 644 ../contrib/moxi.service %{buildroot}%{_unitdir}/moxi.service

%pre
getent group moxi >/dev/null || groupadd -r moxi
getent passwd moxi >/dev/null || \
    useradd -r -g moxi -d /var/lib/moxi -s /sbin/nologin \
    -c "Moxi memcached proxy" moxi
exit 0

%post
%systemd_post moxi.service

%preun
%systemd_preun moxi.service

%postun
%systemd_postun_with_restart moxi.service

%files
%license LICENSE
%doc README ChangeLog
%{_bindir}/moxi
%{_bindir}/vbuckettool
%{_bindir}/vbucketkeygen
%{_libdir}/libconflate.so*
%{_libdir}/libvbucket.so*
%{_libdir}/libmcd.so*
%{_libdir}/libhashkit.so*
%{_unitdir}/moxi.service
%dir %attr(750, moxi, moxi) %{_localstatedir}/lib/moxi

%changelog
* Wed Dec 04 2024 Andrew Kendall <standalone@example.com> - 6.0.0-1
- Major version bump to 6.0.0
- Complete rewrite of CMake build system for standalone building
- Added POSIX platform shim for threading and assertions
- Incorporated upstream fixes from v5.0.0 (MB-25403, MB-24509, CBSE-3717)
- Improved error handling and logging
- Added macOS build support
- Added systemd service file
