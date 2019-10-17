#!/bin/bash

set -e

if [ -z ${MODELS_PATH} ]; then
    echo "please set MODELS_PATH. e.g: export MODELS_PATH=/home/media/workspace/tests"
    exit 1
fi

BASEDIR=$(dirname "$0")/../..
usage="$(basename "$0") [-i <stream>] [-options] -- program to do vehicle and pedestrian classification

where:
-h            show this help text
-a            use hardware decode to accelerate
-i  <stream>  set the stream path
-s            to show on the screen
-v            to show debug log
-d  <devices> set devices for each model(C-CPU G-GPU V-VPU H-HDDL) e.g.CGV"

if [ -z "$1" ]; then
    echo "$usage"
    exit
fi

while getopts ':ahi:svd:' option; do
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
        d) devices_pattern=$OPTARG
            ;;
        \?) printf "illegal option: -%s\n" "$OPTARG" >&2
            echo "$usage" >&2
            exit 1
            ;;
        *)
    esac
done
shift $((OPTIND - 1))

MODEL1=person-vehicle-bike-detection-crossroad-0078
MODEL2=person-attributes-recognition-crossroad-0200
MODEL3=vehicle-attributes-recognition-barrier-0039

CPU=2
GPU=3
VPU=5
HDDL=6
GET_DEVICE_ID() {
    case $1 in
        C)   echo $CPU;;
        G)   echo $GPU;;
        V)   echo $VPU;;
        H)   echo $HDDL;;
        *)   echo Unknown device: $1
        ;;
    esac
}

PRECISION_FP16="\"FP16\""
PRECISION_FP32="\"FP32\""
GET_PRECISION() {
    if [ -z $1 ];then
        exit 0
    fi
    case $1 in
        C)   echo $PRECISION_FP32;;
        G)   echo $PRECISION_FP16;;
        V)   echo $PRECISION_FP16;;
        H)   echo $PRECISION_FP16;;
        *)   echo Unknown device: $1
        ;;
    esac
}

if [ ! -z "$devices_pattern" ]; then
    DEVICE1=$(echo "${devices_pattern:0:1}")
    DEVICE2=$(echo "${devices_pattern:1:1}")
    DEVICE3=$(echo "${devices_pattern:2:1}")
    D_ID1=$(GET_DEVICE_ID $DEVICE1)
    D_ID2=$(GET_DEVICE_ID $DEVICE2)
    D_ID3=$(GET_DEVICE_ID $DEVICE3)
fi
D_ID1=${D_ID1:-$CPU}
D_ID2=${D_ID2:-$CPU}
D_ID3=${D_ID3:-$CPU}

GET_MODEL_PATH() {
    for path in ${MODELS_PATH//:/ }; do
        paths=$(find $path -name "$1*.xml" -print)
        if [ ! -z "$paths" ];
        then
            PRECISION=${2:-\"FP32\"}
            echo $(grep -l "precision=$PRECISION" $paths)
            exit 0
        fi
    done
    echo -e "\e[31mModel $1.xml file was not found. Please set MODELS_PATH\e[0m" 1>&2
    exit 1
}

DETECT_MODEL_PATH=$(GET_MODEL_PATH $MODEL1 $(GET_PRECISION $DEVICE1))
CLASS_MODEL_PATH=$(GET_MODEL_PATH  $MODEL2 $(GET_PRECISION $DEVICE2))
CLASS_MODEL_PATH1=$(GET_MODEL_PATH $MODEL3 $(GET_PRECISION $DEVICE3))

echo "$DETECT_MODEL_PATH"
echo "$CLASS_MODEL_PATH"
echo "$CLASS_MODEL_PATH1"

PROC_PATH() {
    echo ${BASEDIR}/samples/model_proc/$1.json
}

if [ ! -z "$show" ]; then
    $BASEDIR/ffplay $debug_log -i $stream -sync video -vf \
        "detect=model=$DETECT_MODEL_PATH:model_proc=$(PROC_PATH $MODEL1):device=$D_ID1, \
        classify=model=$CLASS_MODEL_PATH:model_proc=$(PROC_PATH $MODEL2):device=$D_ID2, \
        classify=model=$CLASS_MODEL_PATH1:model_proc=$(PROC_PATH $MODEL3):device=$D_ID3, \
        ocv_overlay"
else
    $BASEDIR/ffmpeg $debug_log $hw_accel \
        -i $stream -vf \
        "detect=model=$DETECT_MODEL_PATH:model_proc=$(PROC_PATH $MODEL1):device=$D_ID1, \
        classify=model=$CLASS_MODEL_PATH:model_proc=$(PROC_PATH $MODEL2):device=$D_ID2, \
        classify=model=$CLASS_MODEL_PATH1:model_proc=$(PROC_PATH $MODEL3):device=$D_ID3" \
        -an -f null - #-f iemetadata -y /tmp/vehicle.json
fi

