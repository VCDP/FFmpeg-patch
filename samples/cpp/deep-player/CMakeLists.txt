#*******************************************************************************
#  Copyright (C) 2018-2019 Intel Corporation
#
#  SPDX-License-Identifier: MIT
#*******************************************************************************

cmake_minimum_required (VERSION 3.11)

include(GNUInstallDirs)

if(NOT DEFINED PROJECT_NAME)
  set(NOT_SUBPROJECT ON)
endif()

project(DeepPlayer LANGUAGES CXX VERSION 0.0.1)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

set(FFMPEG_PKG_CONFIG_SUFFIX "" CACHE STRING "This suffix uses for FFmpeg component names searches by pkg-config")

SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")

include_directories(.)

####################################
## to use C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED yes)
####################################

# -pthread sets also some useful compile-time flags
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads)

find_package(FFmpeg
    COMPONENTS AVCODEC AVFORMAT AVUTIL AVDEVICE AVFILTER SWSCALE SWRESAMPLE REQUIRED)

#NEW    include_directories(${FFMPEG_INCLUDE_DIRS})

# OpenCV
find_package(OpenCV)
include_directories(${OpenCV_INCLUDE_DIRS})
if(OpenCV_FOUND)
    add_definitions(-DUSE_OPENCV)
endif()

add_subdirectory(src)
add_subdirectory(sample)
