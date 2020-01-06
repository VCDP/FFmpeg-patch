/*******************************************************************************
 * Copyright (C) 2018-2019 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#pragma once

#include <glog/logging.h>
#include <iostream>
#include <memory>
#include <stdexcept>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/frame.h>
}

struct Frame {
    AVFrame *av_frame;
};

struct Packet {
    AVPacket *av_packet;
};

#define INFO(msg)                                                                                                      \
    fprintf(stderr, "info: %s:%d: ", __FILE__, __LINE__);                                                              \
    fprintf(stderr, "%s\n", msg);
