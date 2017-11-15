#!/bin/bash
mkdir build
cd build

# build and install cmake
# curl -L https://cmake.org/files/v3.10/cmake-3.10.0-rc5.tar.gz --output cmake.tar.gz
# tar xzf cmake.tar.gz
# rm cmake.tar.gz
# mv cmake-* cmake
# cd cmake
# ./configure
# make
# make install
# cd ..

# curl -L https://github.com/libevent/libevent/releases/download/release-2.1.8-stable/libevent-2.1.8-stable.tar.gz --output libevent.tar.gz
# tar xzf libevent.tar.gz
# rm libevent.tar.gz
# mv libevent-* libevent
# cd libevent
# ./configure
# make
# # make verify
# make install
# cd ..

git clone --branch 2.8.2 git://github.com/couchbase/libcouchbase.git
cd libcouchbase && mkdir build
cd build
../cmake/configure
make
ctest
cd ../..

# need to set LD_LIBRARY_PATH to dart-couchbase/tool/build/libcouchbase/build/lib ?
# or copy the files over to some well known path?