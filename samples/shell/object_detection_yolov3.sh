#!/bin/bash
# ==============================================================================
# Copyright (C) 2018-2019 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==============================================================================

set -e

BASEDIR=$(dirname "$0")/../..
if [ -n ${FFVA_SAMPLES_DIR} ]
then
    source $BASEDIR/scripts/setup_env.sh
fi
source $BASEDIR/scripts/setlocale.sh

#import GET_MODEL_PATH
source $BASEDIR/scripts/path_extractor.sh

MODEL=frozen_darknet_yolov3_model

DEVICE=CPU

DETECT_MODEL_PATH=$(GET_MODEL_PATH $MODEL )

if [ -z ${1} ]; then
  echo "ERROR set path to video"
  echo "Usage: ./object_detection_ssd.sh <path/to/your/video/sample>"
  exit
fi

FILE=${1}

ffmpeg -i $FILE \
-vf "detect=model=$DETECT_MODEL_PATH:device=$DEVICE:nireq=8,ocv_overlay" \
-y /tmp/output.h264
