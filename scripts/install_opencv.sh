#!/bin/bash
# ==============================================================================
# Copyright (C) 2018-2019 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==============================================================================

CURDIR=$PWD
cd /tmp/

OPENCV_VER=4.3.0
OPENCV_REPO=https://github.com/opencv/opencv/archive/${OPENCV_VER}.tar.gz
PREFIX=/usr/local

sudo apt-get install -y -q --no-install-recommends libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev python-dev python-numpy

gen_opencv_pc()
{
    cat > opencv.pc <<EOF
# Package Information for pkg-config

prefix=/usr/local
exec_prefix=\${prefix}
libdir=\${exec_prefix}/lib
includedir=\${prefix}

Name: OpenCV
Description: Open Source Computer Vision Library
Version: $OPENCV_VER
Libs: -L\${exec_prefix}/lib -lopencv_dnn -lopencv_ml -lopencv_objdetect -lopencv_stitching -lopencv_calib3d -lopencv_features2d -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_video -lopencv_photo -lopencv_imgproc -lopencv_flann -lopencv_core
Libs.private: -ldl -lm -lpthread -lrt -lva -lva-drm
Cflags: -I\${includedir}
EOF
}

install_opencv()
{
wget ${OPENCV_REPO} && \
    tar -zxvf ${OPENCV_VER}.tar.gz && \
    cd opencv-${OPENCV_VER} && \
    mkdir build && \
    cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$PREFIX -DBUILD_EXAMPLES=OFF -DBUILD_PERF_TESTS=OFF -DBUILD_DOCS=OFF -DBUILD_TESTS=OFF .. && \
    make -j $(nproc) && \
    sudo make install
}

run_install_process()
{
    install_opencv
    gen_opencv_pc
    sudo cp -rf $PREFIX/include/opencv4/opencv2 $PREFIX/include
    sudo cp opencv.pc $PREFIX/lib/pkgconfig
}

run_install_process

cd $CURDIR
