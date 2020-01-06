#!/bin/bash

sudo apt-get install --no-install-recommends libboost-all-dev

CURDIR=$PWD

GLOG_REPO=https://github.com/google/glog.git
GFLAGS_REPO=https://github.com/gflags/gflags.git

git clone $GLOG_REPO third-party/glog
git clone $GFLAGS_REPO third-party/gflags

cd $CURDIR/third-party/glog && git checkout v0.4.0 && git pull origin v0.4.0
mkdir -p build && cd build
cmake -DBUILD_SHARED_LIBS=ON ..
make -j`nproc`
sudo make install


cd $CURDIR/third-party/gflags && git checkout v2.2.2 && git pull origin v2.2.2
mkdir -p build && cd build
cmake -DBUILD_SHARED_LIBS=ON ..
make -j`nproc`
sudo make install

cd $CURDIR
