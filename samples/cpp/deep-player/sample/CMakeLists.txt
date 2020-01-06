# Copyright (C) 2019 Intel Corporation
# SPDX-License-Identifier: Apache-2.0
#

set (TARGET_NAME "deep_play")

find_package(gflags REQUIRED)

file (GLOB MAIN_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
        )

file (GLOB MAIN_HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/*.h
        )

include_directories(${TARGET_NAME} SYSTEM PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/"
)

set(CMAKE_CXX_FLAGS "-std=c++11 -fpermissive ${CMAKE_CXX_FLAGS}")

add_executable(${TARGET_NAME} ${MAIN_SRC} ${MAIN_HEADERS})
target_link_libraries(${TARGET_NAME} deepplayer)
target_link_libraries(${TARGET_NAME} gflags)

install(TARGETS ${TARGET_NAME} DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})
