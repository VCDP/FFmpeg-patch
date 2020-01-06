/*******************************************************************************
 * Copyright (C) 2018-2019 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "render.h"

#ifdef USE_OPENCV
#include "opencv_utils.h"
#endif

extern "C" {
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/frame.h>
#include <libavutil/pixdesc.h>
}

namespace deep_player {

static const char OVERLAY_FILTER_STRING[] = "ocv_overlay";

Render::Overlay::Overlay(AVFrame *input, AVRational time_base, const char *filters_desc) {
    int ret = 0;

    if (input == nullptr || filters_desc == nullptr)
        throw std::runtime_error("Bad init params!");

    const AVFilter *buffersrc = avfilter_get_by_name("buffer");
    const AVFilter *buffersink = avfilter_get_by_name("buffersink");
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs = avfilter_inout_alloc();
    mFilterGraph = avfilter_graph_alloc();
    if (outputs == nullptr || inputs == nullptr || !mFilterGraph) {
        throw std::runtime_error("Alloc filter graph failed!");
    }

    char args[512];
    /* buffer video source: the decoded frames from the decoder will be inserted here. */
    snprintf(args, sizeof(args), "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d", input->width,
             input->height, input->format, time_base.num, time_base.den, input->sample_aspect_ratio.num,
             input->sample_aspect_ratio.den);

    ret = avfilter_graph_create_filter(&mBufferSrcCtx, buffersrc, "in", args, NULL, mFilterGraph);
    if (ret < 0) {
        throw std::runtime_error("Cannot create buffer source\n");
    }
    /* buffer video sink: to terminate the filter chain. */
    ret = avfilter_graph_create_filter(&mBufferSinkCtx, buffersink, "out", NULL, NULL, mFilterGraph);
    if (ret < 0) {
        throw std::runtime_error("Cannot create buffer sink\n");
    }

    outputs->name = av_strdup("in");
    outputs->filter_ctx = mBufferSrcCtx;
    outputs->pad_idx = 0;
    outputs->next = NULL;

    inputs->name = av_strdup("out");
    inputs->filter_ctx = mBufferSinkCtx;
    inputs->pad_idx = 0;
    inputs->next = NULL;

    ret = avfilter_graph_parse_ptr(mFilterGraph, filters_desc, &inputs, &outputs, NULL);
    if (ret < 0) {
        throw std::runtime_error("Parse filter graph error!\n");
    }

    ret = avfilter_graph_config(mFilterGraph, NULL);
    if (ret < 0) {
        throw std::runtime_error("Config filter graph error!\n");
    }

    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);
}

Render::Overlay::~Overlay() {
    avfilter_graph_free(&mFilterGraph);
}

void Render::Overlay::FilterFrame(AVFrame *frame, AVFrame *filt_frame) {
    int ret = av_buffersrc_add_frame_flags(mBufferSrcCtx, frame, AV_BUFFERSRC_FLAG_KEEP_REF);
    if (ret < 0)
        throw std::runtime_error("Error while feeding the filtergraph\n");

    while (ret >= 0) {
        ret = av_buffersink_get_frame(mBufferSinkCtx, filt_frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        }

        if (ret < 0) {
            throw std::runtime_error("Error while getting the result \n");
        }
    }
}

void Render::Init(const AVFormatContext *fmt_ctx, size_t index, const bounded_buffer<Frame>::Ptr &frameBufQueue,
                  bool noShow, int freshRate) {
    if (fmt_ctx == nullptr || frameBufQueue.get() == nullptr) {
        throw std::runtime_error("Bad init params for render!");
    }

    mFrameQueue = frameBufQueue;
    if (freshRate > 0)
        mRenderTimePerFrame = static_cast<int>(1000.0f / freshRate);

    mTimeBase = fmt_ctx->streams[index]->time_base;

    mNoShow = noShow;
    mInitialized = true;

    mTimer = new Timer();
}

void Render::Start() {
    if (!mInitialized)
        throw std::logic_error("Missing render init steps!");

    mRunning = true;
    mTimer->start("total");
    mWorkingThread = std::thread(&Render::WorkingFunction, this);
}

void Render::flush() {
    if (!mRunning)
        return;

    mFrameQueue->wait_until_empty();
    Frame last_frame = {0};
    mFrameQueue->push_front(last_frame);
}

void Render::Stop() noexcept {
    if (!mRunning)
        return;

    if (mWorkingThread.joinable()) {
        flush();
        mRunning = false;
        mWorkingThread.join();
    }

    mTimer->finish("total");
    std::cout << "Number of processed frames: " << mFramesCounter << std::endl;
    std::cout << "Total image throughput: " << mFramesCounter * (1000.f / (*mTimer)["total"].getTotalDuration())
              << " fps" << std::endl;
}

Render::~Render() {
    if (mOverlay)
        delete mOverlay;
    if (mTimer)
        delete mTimer;
    mInitialized = false;
}

bool Render::IsEscKey() noexcept {
    return mKey == 27; // 27: ESC
}

void Render::WorkingFunction() {
    Frame frame_display = {0};

    AVFrame *filt_frame = av_frame_alloc();

    for (;;) {
        if (!mRunning)
            break;

        mFrameQueue->pop_back(&frame_display);
        AVFrame *video_frame = frame_display.av_frame;

        // empty Frame indicates end of stream
        if (video_frame == nullptr) {
            break;
        }

        mTimer->start("one");

        const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get((AVPixelFormat)video_frame->format);
        assert(desc);

        if (desc->flags & AV_PIX_FMT_FLAG_HWACCEL) {
            AVFrame *sw_frame = av_frame_alloc();
            assert(sw_frame);

            int ret = av_hwframe_transfer_data(sw_frame, video_frame, 0);
            if (ret < 0)
                throw std::runtime_error("Error transferring the data to system memory");

            auto clone_side_data = [](AVFrame *frame, AVFrameSideData *sd, int type) {
                if (!sd)
                    return;

                AVBufferRef *_ref = av_buffer_ref(sd->buf);
                assert(_ref);

                AVFrameSideData *ret =
                    av_frame_new_side_data_from_buf(frame, static_cast<AVFrameSideDataType>(type), _ref);
                if (ret == nullptr) {
                    av_buffer_unref(&_ref);
                    throw std::runtime_error("Error moving the side data");
                }
            };

            AVFrameSideData *detect_sd = av_frame_get_side_data(video_frame, AV_FRAME_DATA_INFERENCE_DETECTION);
            AVFrameSideData *classify_sd = av_frame_get_side_data(video_frame, AV_FRAME_DATA_INFERENCE_CLASSIFICATION);
            clone_side_data(sw_frame, detect_sd, AV_FRAME_DATA_INFERENCE_DETECTION);
            clone_side_data(sw_frame, classify_sd, AV_FRAME_DATA_INFERENCE_CLASSIFICATION);

            av_frame_free(&video_frame);
            video_frame = sw_frame;
        }

        mFramesCounter++;

        if (!mNoShow) {
            if (mOverlay == nullptr)
                mOverlay = new Overlay(video_frame, mTimeBase, OVERLAY_FILTER_STRING);

            mOverlay->FilterFrame(video_frame, filt_frame);

            cv::Mat mat;
            CVUtils::AVFrameToMat(filt_frame, mat);

            std::ostringstream out;
            out.str("");
            out << "Total image throughput: " << std::fixed << std::setprecision(2)
                << mFramesCounter * 1000.f / ((*mTimer)["total"].getDurationSinceCallStart()) << " fps";
            cv::putText(mat, out.str(), cv::Point2f(10, 45), cv::FONT_HERSHEY_TRIPLEX, 1.2, cv::Scalar(255, 0, 0), 2);

            cv::imshow("Video", mat);

            mTimer->finish("one");

            int delay = 1;
            if (mRenderTimePerFrame > 0)
                delay = std::max(1, static_cast<int>(mRenderTimePerFrame - (*mTimer)["one"].getLastCallDuration()));

            int key = cv::waitKey(delay);
            if (key >= 0)
                mKey = key;
        }

        av_frame_unref(filt_frame);
        av_frame_free(&video_frame);
    }

    av_frame_free(&filt_frame);
}

CallStat::CallStat() : _total_duration(0.0), _last_call_duration(0.0), _smoothed_duration(-1.0) {
}

double CallStat::getSmoothedDuration() {
    // Additional check is needed for the first frame while duration of the first
    // visualisation is not calculated yet.
    if (_smoothed_duration < 0) {
        auto t = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<ms>(t - _last_call_start).count();
    }
    return _smoothed_duration;
}

double CallStat::getTotalDuration() {
    return _total_duration;
}

double CallStat::getLastCallDuration() {
    return _last_call_duration;
}

double CallStat::getDurationSinceCallStart() {
    auto t = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration_cast<ms>(t - _last_call_start).count();
    return duration;
}

void CallStat::calculateDuration() {
    auto t = std::chrono::high_resolution_clock::now();
    _last_call_duration = std::chrono::duration_cast<ms>(t - _last_call_start).count();
    _total_duration += _last_call_duration;
    if (_smoothed_duration < 0) {
        _smoothed_duration = _last_call_duration;
    }
    double alpha = 0.1;
    _smoothed_duration = _smoothed_duration * (1.0 - alpha) + _last_call_duration * alpha;
}

void CallStat::setStartTime() {
    _last_call_start = std::chrono::high_resolution_clock::now();
}

void Timer::start(const std::string &name) {
    if (_timers.find(name) == _timers.end()) {
        _timers[name] = CallStat();
    }
    _timers[name].setStartTime();
}

void Timer::finish(const std::string &name) {
    auto &timer = (*this)[name];
    timer.calculateDuration();
}

CallStat &Timer::operator[](const std::string &name) {
    if (_timers.find(name) == _timers.end()) {
        throw std::logic_error("No timer with name " + name + ".");
    }
    return _timers[name];
}

} // namespace deep_player