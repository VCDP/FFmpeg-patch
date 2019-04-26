#!/bin/bash

set -e

if [ -z ${MODELS_PATH} ]; then
    echo "please set MODELS_PATH. e.g: export MODELS_PATH=/home/media/workspace/tests"
    exit 1
fi

BASEDIR=$(dirname "$0")/../..
usage="$(basename "$0") [-h] [-i <stream>] [-s] -- program to do face classification

where:
-h           show this help text
-a           use hardware decode to accelerate
-i  <stream> set the stream path
-s           to show on the screen
-v           to show debug log"

if [ -z "$1" ]; then
    echo "$usage"
    exit
fi

while getopts ':ahi:sv' option; do
    case "$option" in
        h) echo "$usage"
            exit
            ;;
        a) hw_accel="-flags unaligned -hwaccel vaapi -hwaccel_output_format vaapi -hwaccel_device /dev/dri/renderD128"
            ;;
        i) stream=$OPTARG
            ;;
        s) show="true"
            ;;
        v) debug_log="-loglevel debug"
            ;;
        \?) printf "illegal option: -%s\n" "$OPTARG" >&2
            echo "$usage" >&2
            exit 1
            ;;
        *)
    esac
done
shift $((OPTIND - 1))

MODEL1=vehicle-license-plate-detection-barrier-0106
MODEL2=vehicle-attributes-recognition-barrier-0039
MODEL3=license-plate-recognition-barrier-0001

PRECISION=${2:-\"FP32\"}

GET_MODEL_PATH() {
    for path in ${MODELS_PATH//:/ }; do
        paths=$(find $path -name "$1.xml" -print)
        if [ ! -z "$paths" ];
        then
            echo $(grep -l "precision=$PRECISION" $paths)
            exit 0
        fi
    done
    echo -e "\e[31mModel $1.xml file was not found. Please set MODELS_PATH\e[0m" 1>&2
    exit 1
}

DETECT_MODEL_PATH=$(GET_MODEL_PATH $MODEL1)
CLASS_MODEL_PATH=$(GET_MODEL_PATH $MODEL2)
CLASS_MODEL_PATH1=$(GET_MODEL_PATH $MODEL3)

echo "$DETECT_MODEL_PATH"
echo "$CLASS_MODEL_PATH"
echo "$CLASS_MODEL_PATH1"

PROC_PATH() {
    echo ${BASEDIR}/samples/model_proc/$1.json
}

if [ ! -z "$show" ]; then
    $BASEDIR/ffplay $debug_log -i $stream -sync video -vf "detect=model=$DETECT_MODEL_PATH:model_proc=$(PROC_PATH $MODEL1), \
        classify=model=$CLASS_MODEL_PATH:model_proc=$(PROC_PATH $MODEL2), \
        classify=model=$CLASS_MODEL_PATH1:model_proc=$(PROC_PATH $MODEL3), \
        ocv_overlay"
else
    $BASEDIR/ffmpeg $debug_log $hw_accel \
        -i $stream -vf "detect=model=$DETECT_MODEL_PATH:model_proc=$(PROC_PATH $MODEL1), \
        classify=model=$CLASS_MODEL_PATH:model_proc=$(PROC_PATH $MODEL2),  \
        classify=model=$CLASS_MODEL_PATH1:model_proc=$(PROC_PATH $MODEL3)" \
        -f null -
fi
