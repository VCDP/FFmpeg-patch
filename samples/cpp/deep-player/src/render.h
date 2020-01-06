/*******************************************************************************
 * Copyright (C) 2018-2019 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#pragma once

#include "bounded_buffer.h"
#include "common.h"
#include <chrono>
#include <map>
#include <thread>

extern "C" {
#include <libavfilter/avfilter.h>
#include <libavformat/avformat.h>
}

namespace deep_player {
class CallStat;
class Timer;

class Render {
  public:
    using Ptr = std::shared_ptr<Render>;

    Render() = default;
    virtual ~Render();

    virtual void Init(const AVFormatContext *fmt_ctx, size_t index, const bounded_buffer<Frame>::Ptr &frameBufQueue,
                      bool noShow = false, int freshRate = 0);

    virtual void Start();
    virtual void Stop() noexcept;

    virtual bool IsEscKey() noexcept;

    class Overlay {
      public:
        explicit Overlay(AVFrame *input, AVRational time_base, const char *filters_desc);
        Overlay(AVFrame *input_frame, const char *filter_desc);
        virtual ~Overlay();

        void FilterFrame(AVFrame *frame, AVFrame *filt_frame);

      private:
        AVFilterContext *mBufferSinkCtx = nullptr;
        AVFilterContext *mBufferSrcCtx = nullptr;
        AVFilterGraph *mFilterGraph = nullptr;
    };

  protected:
    void WorkingFunction();

  private:
    bool mInitialized = false;
    bool mRunning = false;
    bool mNoShow = false;
    int mKey = -1;
    int mRenderTimePerFrame = 30;
    size_t mFramesCounter = 0;

    AVRational mTimeBase;
    Overlay *mOverlay = nullptr;
    Timer *mTimer = nullptr;

    void flush();

    std::thread mWorkingThread;

    bounded_buffer<Frame>::Ptr mFrameQueue;
};

class CallStat {
  public:
    typedef std::chrono::duration<double, std::ratio<1, 1000>> ms;

    CallStat();

    double getSmoothedDuration();
    double getTotalDuration();
    double getLastCallDuration();
    double getDurationSinceCallStart();
    void calculateDuration();
    void setStartTime();

  private:
    double _total_duration;
    double _last_call_duration;
    double _smoothed_duration;
    std::chrono::time_point<std::chrono::high_resolution_clock> _last_call_start;
};

class Timer {
  public:
    void start(const std::string &name);
    void finish(const std::string &name);
    CallStat &operator[](const std::string &name);

  private:
    std::map<std::string, CallStat> _timers;
};

} // namespace deep_player