/*******************************************************************************
 * Copyright (C) 2018-2019 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#pragma once

#include "bounded_buffer.h"
#include "common.h"
#include "demuxer.h"
#include <thread>

extern "C" {
#include <libavcodec/avcodec.h>
}

namespace deep_player {

class Decoder {
  public:
    using Ptr = std::shared_ptr<Decoder>;

    Decoder();
    virtual ~Decoder();

    virtual void SetAVPacketBufferQueue(const bounded_buffer<Packet>::Ptr &avPacketBufQueue) noexcept;

    virtual void SetAVFrameBufferQueue(const bounded_buffer<Frame>::Ptr &avFrameBufQueue) noexcept;

    virtual void Init(const Demuxer::Ptr &demuxer, const AVCodec *codec, const char *deviceName, int extra_hw_frames);

    virtual AVPixelFormat GetHWPixelFormat() const noexcept {
        return mHWPixelFormat;
    }

    virtual AVCodecContext *GetAVCodecContext() const noexcept {
        return mAVCodecContext;
    }

    virtual void Start();
    virtual void Stop() noexcept;

  protected:
    void WorkingFunction();

  private:
    bool mInitialized;

    Demuxer::Ptr mDemux;

    std::thread mWorkingThread;

    bounded_buffer<Packet>::Ptr mPacketQueue;
    bounded_buffer<Frame>::Ptr mFrameQueue;

    AVCodecContext *mAVCodecContext;
    AVBufferRef *mHWDeviceCtx;
    AVPixelFormat mHWPixelFormat;

    bool init_hw_context(const AVCodec *decoder, const char *name, int extra_hw_frames);
    int decode(AVFrame *frame);
};

} // namespace deep_player