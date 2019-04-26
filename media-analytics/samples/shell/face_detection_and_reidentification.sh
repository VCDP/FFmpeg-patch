#!/bin/bash

set -e

if [ -z ${MODELS_PATH} ]; then
    echo "please set MODELS_PATH. e.g: export MODELS_PATH=/home/media/workspace/tests"
    exit 1
fi

BASEDIR=$(dirname "$0")/../..
usage="$(basename "$0") [-i <stream>] [-options] -- program to do face reidentification

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

MODEL1=face-detection-retail-0004
MODEL2=face-reidentification-retail-0095

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

TEST=${2:-\"FP16\"}
GET_MODEL_PATH2() {
    for path in ${MODELS_PATH//:/ }; do
        paths=$(find $path -name "$1.xml" -print)
        if [ ! -z "$paths" ];
        then
            echo $(grep -l "precision=$TEST" $paths)
            exit 0
        fi
    done
    echo -e "\e[31mModel $1.xml file was not found. Please set MODELS_PATH\e[0m" 1>&2
    exit 1
}

DETECT_MODEL_PATH=$(GET_MODEL_PATH $MODEL1)
CLASS_MODEL_PATH=$(GET_MODEL_PATH2 $MODEL2)

echo "$DETECT_MODEL_PATH"
echo "$CLASS_MODEL_PATH"

PROC_PATH() {
    echo ${BASEDIR}/samples/model_proc/$1.json
}

# !!!! please change to your path
FEATURE=$MODELS_PATH/face-reid-features/vec_to_bin/registered_faces.bin
LABEL=$MODELS_PATH/face-reid-features/face_reidentification_demo/face_gallery/labels.txt

if [ ! -e $FEATURE ]; then
echo "please change FEATURE to your path"
exit 1
fi

if [ ! -e $LABEL ]; then
echo "please change LABEL to your path"
exit 1
fi

if [ ! -z "$show" ]; then
    $BASEDIR/ffplay $debug_log -i $stream -sync video -vf "detect=model=$DETECT_MODEL_PATH, \
        classify=model=$CLASS_MODEL_PATH:label=$LABEL:feature_file=$FEATURE:name=face_reid, \
        ocv_overlay"
else
    $BASEDIR/ffmpeg $debug_log $hw_accel \
        -i $stream -vf "detect=model=$DETECT_MODEL_PATH, \
        classify=model=$CLASS_MODEL_PATH:label=$LABEL:feature_file=$FEATURE:name=face_reid:device=3" \
        -f iemetadata -y /tmp/face-identify.json
fi
