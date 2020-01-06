/*******************************************************************************
 * Copyright (C) 2018-2019 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#pragma once

#include "bounded_buffer.h"
#include "common.h"
#include "input_source.h"
#include <thread>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

namespace deep_player {

class Demuxer {
  public:
    using Ptr = std::shared_ptr<Demuxer>;

    Demuxer();
    virtual ~Demuxer() = default;

    virtual void Init(const InputSource::Ptr &input);

    virtual void SetAVPacketBufferQueue(const bounded_buffer<Packet>::Ptr &avPacketBufQueue) noexcept;

    virtual const int GetVideoStreamIndex() const {
        return mVideoStreamIndex;
    }

    virtual const AVCodec *GetAVCodec() const {
        return mAVCodec;
    }

    virtual const AVCodecParameters *GetVideoCodecParameters() const {
        if (!mInitialized)
            return nullptr;

        return mAVFormatContext->streams[mVideoStreamIndex]->codecpar;
    }

    virtual bool EOS() const {
        return mEOSReached;
    }

    virtual void Start();
    virtual void Stop();

  protected:
    void WorkingFunction();

  private:
    std::thread mWorkingThread;
    bool mRunning = false;
    bool mInitialized = false;
    bool mEOSReached = false;

    bounded_buffer<Packet>::Ptr mPacketQueue;

    InputSource::Ptr mInput;
    int mVideoStreamIndex = -1;

    AVCodec *mAVCodec = nullptr;
    AVFormatContext *mAVFormatContext = nullptr;
};

} // namespace deep_player