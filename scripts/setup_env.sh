#!/bin/bash
# ==============================================================================
# Copyright (C) 2018-2019 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==============================================================================

export FFVA_SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
export FFVA_HOME=${FFVA_SCRIPT_DIR}/..
export FFVA_SAMPLES_DIR=${FFVA_HOME}/samples

DEFAULT_VIDEO_DIR=${FFVA_HOME}/../data/video
export VIDEO_EXAMPLES_DIR=${VIDEO_EXAMPLES_DIR:-$DEFAULT_VIDEO_DIR}

DEFAULT_MODELS_PATH=${FFVA_HOME}/../data/models
export MODELS_PATH=${MODELS_PATH:-$DEFAULT_MODELS_PATH}

echo [setup_env.sh] FFmpeg-analytics-plugins environment initialized
