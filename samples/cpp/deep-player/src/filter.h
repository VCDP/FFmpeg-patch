/*******************************************************************************
 * Copyright (C) 2018-2019 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#pragma once

#include "bounded_buffer.h"
#include "common.h"
#include "decoder.h"
#include <thread>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavformat/avformat.h>
}

namespace deep_player {

class Filter {
  public:
    using Ptr = std::shared_ptr<Filter>;

    Filter() = default;
    virtual ~Filter();

    virtual void Init(const AVFormatContext *fmt_ctx, size_t index, const AVCodecContext *dec_ctx,
                      AVPixelFormat hw_pix_fmt, const char *filters_desc, const char *name);

    virtual void SetInputAVFrameBufferQueue(const bounded_buffer<Frame>::Ptr &frameBufQueue) noexcept {
        mInputFrameQueue = frameBufQueue;
    }

    virtual void SetOutputAVFrameBufferQueue(const bounded_buffer<Frame>::Ptr &frameBufQueue) noexcept {
        mOutputFrameQueue = frameBufQueue;
    }

    virtual void Start();
    virtual void Stop() noexcept;

  protected:
    void WorkingFunction();

  private:
    bool mInitialized = false;
    bool mRunning = false;
    std::string mName;

    void flush();

    Decoder::Ptr mDecoder;

    std::thread mWorkingThread;

    bounded_buffer<Frame>::Ptr mInputFrameQueue;
    bounded_buffer<Frame>::Ptr mOutputFrameQueue;

    AVFilterContext *mBufferSinkCtx = nullptr;
    AVFilterContext *mBufferSrcCtx = nullptr;
    AVFilterGraph *mFilterGraph = nullptr;
    AVPixelFormat mHWPixelFormat = AV_PIX_FMT_NONE;
    AVPixelFormat mSWPixelFormat = AV_PIX_FMT_NONE;
};

} // namespace deep_player