/*******************************************************************************
 * Copyright (C) 2018-2019 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#pragma once

#include "common.h"
#include <string>

extern "C" {
#include <libavformat/avformat.h>
}

namespace deep_player {

class InputSource {
  public:
    using Ptr = std::shared_ptr<InputSource>;

    InputSource() = default;
    explicit InputSource(const std::string &uri);
    virtual ~InputSource();

    virtual AVFormatContext *GetAVFormatContext() const noexcept {
        return mAVFormatCtx;
    }

    virtual const std::string &GetURI() const noexcept {
        return mUri;
    }

  private:
    std::string mUri;
    AVFormatContext *mAVFormatCtx = nullptr;
};

} // namespace deep_player