#!/bin/bash
# ==============================================================================
# Copyright (C) 2018-2019 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==============================================================================

# If you use MSS install package, please add following VAR into ENV
LIBVA_PKG_CONFIG_PATH="/opt/intel/mediasdk/lib64/pkgconfig"
PKG_CONFIG_PATH="${PKG_CONFIG_PATH}:$LIBVA_PKG_CONFIG_PATH"

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
    --enable-libinference_engine_c_wrapper
# appending options here to enable more third party components

make -j16 && make install
