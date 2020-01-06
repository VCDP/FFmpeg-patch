/*******************************************************************************
 * Copyright (C) 2018-2019 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "common.h"

extern "C" {
#include <libavutil/frame.h>
#include <libswscale/swscale.h>
}

#include <opencv2/opencv.hpp>

namespace deep_player {

class CVUtils {
  public:
    static void AVFrameToMat(AVFrame *frame, cv::Mat &image);
};

} // namespace deep_player