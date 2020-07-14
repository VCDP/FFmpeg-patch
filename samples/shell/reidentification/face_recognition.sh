#!/bin/bash
# ==============================================================================
# Copyright (C) 2018-2019 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==============================================================================

set -e

BASEDIR=$(dirname "$0")/../../..
if [ -n ${FFVA_SAMPLES_DIR} ]
then
    source $BASEDIR/scripts/setup_env.sh
fi
source $BASEDIR/scripts/setlocale.sh
#import GET_MODEL_PATH and PROC_PATH
source $BASEDIR/scripts/path_extractor.sh

if [ -z ${1} ]; then
  echo "ERROR set path to video"
  echo "Usage: ./face_recognition.sh <path/to/your/video/sample>"
  exit
fi

INPUT=${1}
GALLERY=$FFVA_SAMPLES_DIR/shell/reidentification/gallery/gallery.json

DETECTION_MODEL=face-detection-adas-0001
IDENTIFICATION_MODEL=face-reidentification-retail-0095

IDENTIFICATION_MODEL_PROC=face-reidentification-retail-0095

DEVICE=CPU

DETECT_MODEL_PATH=$(GET_MODEL_PATH $DETECTION_MODEL )
IDENTIFICATION_MODEL_PATH=$(GET_MODEL_PATH $IDENTIFICATION_MODEL )

ffmpeg -i $INPUT -vf "detect=model=$DETECT_MODEL_PATH:device=$DEVICE:nireq=4, \
classify=model=$IDENTIFICATION_MODEL_PATH:model_proc=$(PROC_PATH $IDENTIFICATION_MODEL_PROC):device=$DEVICE:nireq=4, \
identify=gallery=$GALLERY" \
-an -f metapublish -output_format batch -y /tmp/face-identify.json

