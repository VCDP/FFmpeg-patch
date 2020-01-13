#!/bin/bash
# ==============================================================================
# Copyright (C) 2018-2019 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==============================================================================

set -e

BASEDIR=$(dirname "$0")/../../..

if [ -n ${FFVA_SAMPLES_DIR} ]; then
  source $BASEDIR/scripts/setup_env.sh
fi

source $BASEDIR/scripts/setlocale.sh
source $BASEDIR/scripts/path_extractor.sh

if [[ -z ${1} || -z ${2} ]]; then
  echo "ERROR set path to video or set path to output"
  echo "Usage: ./face_metapublish_file.sh <path/to/your/video/sample> <path/to/output/json/file>"
  exit
fi

INPUT=${1}
OUTFILE=${2}

MODEL1=face-detection-adas-0001
MODEL2=age-gender-recognition-retail-0013
MODEL3=emotions-recognition-retail-0003
MODEL1_PROC=face-detection-adas-0001
MODEL2_PROC=age-gender-recognition-retail-0013
MODEL3_PROC=emotions-recognition-retail-0003

DEVICE="CPU"

DETECT_MODEL_PATH=$(GET_MODEL_PATH $MODEL1)
CLASS_MODEL_PATH=$(GET_MODEL_PATH $MODEL2)
CLASS_MODEL_PATH1=$(GET_MODEL_PATH $MODEL3)

if [[ $INPUT == "/dev/video"* ]]; then
  SOURCE="-f v4l2 -i ${INPUT}"
else
  SOURCE="-i ${INPUT}"
fi

# optional
CUSTOM_TAG="'{\"segmentation\"\: 10000}'"

ffmpeg $SOURCE -vf \
    "detect=model=$DETECT_MODEL_PATH:model_proc=$(PROC_PATH $MODEL1):device=$DEVICE, \
    classify=model=$CLASS_MODEL_PATH:model_proc=$(PROC_PATH $MODEL2):device=$DEVICE, \
    classify=model=$CLASS_MODEL_PATH1:model_proc=$(PROC_PATH $MODEL3):device=$DEVICE, \
    metaconvert=converter=json:method=all:source=$INPUT:tags=$CUSTOM_TAG" \
    -vframes 100 \
    -an -y -f metapublish -output_format batch $OUTFILE
