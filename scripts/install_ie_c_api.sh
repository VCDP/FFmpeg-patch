#!/bin/bash
# ==============================================================================
# Copyright (C) 2018-2019 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==============================================================================

CURDIR=$PWD
cd /tmp/

#make sure you have openvino built or installed
source  /opt/intel/openvino/bin/setupvars.sh

tar -zxvf $CURDIR/../thirdparty/dldt-c-api/source/v2.0.0.tar.gz && \
    cd dldt-c_api-2.0.0 && \
    mkdir -p build && cd build && \
    cmake -DENABLE_AVX512F=OFF .. && \
    make -j8 && \
    sudo make install

cd $CURDIR