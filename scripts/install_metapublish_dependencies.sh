#!/bin/bash
# ==============================================================================
# Copyright (C) 2018-2019 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==============================================================================

CURDIR=$PWD
cd /tmp/

wget -O - https://github.com/edenhill/librdkafka/archive/v1.0.0.tar.gz | tar -xz
cd librdkafka-1.0.0
./configure --prefix=/usr --libdir=/usr/lib/x86_64-linux-gnu/
make
sudo make install
cd ..
sudo rm -rf librdkafka-1.0.0

cd $CURDIR
