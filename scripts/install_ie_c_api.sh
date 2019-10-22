#!/bin/bash
# ==============================================================================
# Copyright (C) 2018-2019 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==============================================================================

CURDIR=$PWD
cd /tmp/

tar -zxvf $CURDIR/../thirdparty/dldt-c-api/source/dldt-c_api_v2-1.0.tar.gz && \
    cd dldt-c_api-1.0 && \
    mkdir -p build && cd build && \
    cmake -DENABLE_AVX512F=OFF .. && \
    make -j8 && \
    sudo make install

cd $CURDIR