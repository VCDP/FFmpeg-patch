#*******************************************************************************
# Copyright (C) 2018-2019 Intel Corporation
#
# SPDX-License-Identifier: MIT
#******************************************************************************/

include_directories(.)

file(GLOB_RECURSE AV_SOURCES "*.cc" "*.cpp")
file(GLOB_RECURSE AV_HEADERS "*.h" "*.hpp")

set(AV_TARGET deepplayer)

find_package(Boost REQUIRED)
find_package(glog 0.4.0 REQUIRED)

add_library(${AV_TARGET}  SHARED ${AV_SOURCES})

target_link_libraries(${AV_TARGET}
    Threads::Threads
    FFmpeg::FFmpeg
    boost_system
    boost_thread
    glog::glog
)

set(PLATFORM_LIBS
    ${CMAKE_THREAD_LIBS_INIT}
    ${FFMPEG_LIBRARIES})

target_include_directories(${AV_TARGET}
  PUBLIC
    ${Boost_INCLUDE_DIR}
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(OpenCV_FOUND)
    target_link_libraries(${AV_TARGET} ${OpenCV_LIBRARIES})
endif()

# provide a namespaced alias for clients to 'link' against if catch is included as a sub-project
add_library(deepPlay::deepPlay ALIAS ${AV_TARGET})

install(TARGETS ${AV_TARGET} DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})
