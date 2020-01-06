/*******************************************************************************
 * Copyright (C) 2018-2019 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "opencv_utils.h"

namespace deep_player {

// Convert to OpenCV image (BGR24)
void CVUtils::AVFrameToMat(AVFrame *frame, cv::Mat &image) {
    if (frame == nullptr)
        return;

    int width = frame->width;
    int height = frame->height;

    if (image.rows != height || image.cols != width || image.type() != CV_8UC3)
        image = cv::Mat(height, width, CV_8UC3);

    static SwsContext *sws_context =
        sws_getCachedContext(sws_context, width, height, (AVPixelFormat)frame->format, width, height, AV_PIX_FMT_BGR24,
                             SWS_BICUBIC, NULL, NULL, NULL);
    int linestep = image.step;
    sws_scale(sws_context, frame->data, frame->linesize, 0, frame->height, &image.data, &linestep);
}

} // namespace deep_player