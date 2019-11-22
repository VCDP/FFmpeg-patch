#!/bin/bash
# ==============================================================================
# Copyright (C) 2018-2019 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==============================================================================

build_type=${1:-package}
tag=${2:-latest}

if [ $build_type == "opensource" ]; then
  dockerfile=Dockerfile.source
elif [ $build_type == "package" ]; then
  dockerfile=Dockerfile.package
else
  echo "Usage: ./build_docker_image.sh [BUILDTYPE] [TAG]"
  echo "ERROR: please set BUILDTYPE to on of the following: [opensource, package]"
  exit
fi

BASEDIR=$(dirname "$0")

# image:build
sudo docker build --target build -f ${BASEDIR}/${dockerfile} --network=host \
    -t "ffmpeg-video-analytics-$build_type:build" "$BASEDIR" $(env | grep -E '_(proxy|REPO|VER)=' | sed 's/^/--build-arg /')

sudo docker build -f ${BASEDIR}/${dockerfile} --network=host -t ffmpeg-video-analytics-$build_type:$tag \
    --build-arg http_proxy=${http_proxy} \
    --build-arg https_proxy=${https_proxy} \
    ${BASEDIR}
