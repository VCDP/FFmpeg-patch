#!/bin/bash
# ==============================================================================
# Copyright (C) 2018-2019 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==============================================================================

./configure \
    --extra-libs="-lpthread -lm" \
    --enable-shared \
    --enable-gpl    \
    --enable-openssl \
    --enable-nonfree \
    --enable-libdrm  \
    --disable-doc --disable-htmlpages --disable-manpages --disable-podpages --disable-txtpages \
    --enable-libx264 \
    --enable-libx265 \
    --enable-librdkafka \
    --enable-libjson_c \
    --enable-libinference_engine_c_api
# appending options here to enable more third party components

make -j16 && make install
