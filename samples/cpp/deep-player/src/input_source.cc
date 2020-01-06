/*******************************************************************************
 * Copyright (C) 2018-2019 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "input_source.h"
#include "common.h"

namespace deep_player {

InputSource::InputSource(const std::string &uri) : mAVFormatCtx(nullptr) {
    int ret = avformat_open_input(&mAVFormatCtx, uri.empty() ? nullptr : uri.c_str(), NULL, NULL);
    if (ret < 0)
        throw std::runtime_error("open input failed!\n");

    ret = avformat_find_stream_info(mAVFormatCtx, nullptr);
    if (ret < 0)
        throw std::runtime_error("find stream info failed!\n");

    mUri = uri;
}

InputSource::~InputSource() {
    if (mAVFormatCtx)
        avformat_close_input(&mAVFormatCtx);
}

} // namespace deep_player