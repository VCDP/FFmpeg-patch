#!/bin/bash
# ==============================================================================
# Copyright (C) 2018-2019 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==============================================================================

set -e

BASEDIR=$(dirname "$0")/../..
if [ -n ${FFVA_SAMPLES_DIR} ]; then
  source $BASEDIR/scripts/setup_env.sh
fi
source $BASEDIR/scripts/setlocale.sh
source $BASEDIR/scripts/path_extractor.sh

if [ -z ${1} ]; then
  echo "ERROR set path to video"
  echo "Usage: ./face_detection_and_classification.sh <path/to/your/video/sample>"
  exit
fi

INPUT=${1}

MODEL1=face-detection-adas-0001
MODEL2=age-gender-recognition-retail-0013
MODEL3=emotions-recognition-retail-0003
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

ffplay $SOURCE -sync video -vf \
  "detect=model=$DETECT_MODEL_PATH:device=$DEVICE, \
   classify=model=$CLASS_MODEL_PATH:model_proc=$(PROC_PATH $MODEL2_PROC):device=$DEVICE, \
   classify=model=$CLASS_MODEL_PATH1:model_proc=$(PROC_PATH $MODEL3_PROC):device=$DEVICE, \
   ocv_overlay"
