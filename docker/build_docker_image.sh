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
sudo docker build -f ${BASEDIR}/${dockerfile} --network=host -t ffmpeg-video-analytics:$tag \
    --build-arg http_proxy=${HTTP_PROXY} \
    --build-arg https_proxy=${HTTPS_PROXY} \
    ${BASEDIR}
