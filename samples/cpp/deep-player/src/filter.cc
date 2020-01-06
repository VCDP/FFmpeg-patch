/*******************************************************************************
 * Copyright (C) 2018-2019 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "filter.h"

#ifdef USE_OPENCV
#include "opencv_utils.h"
#endif

extern "C" {
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
}

namespace deep_player {

Filter::~Filter() {
    avfilter_graph_free(&mFilterGraph);
    mInitialized = false;
}

void Filter::Init(const AVFormatContext *fmt_ctx, size_t index, const AVCodecContext *dec_ctx, AVPixelFormat hw_pix_fmt,
                  const char *filters_desc, const char *name) {
    int ret = 0;

    if (fmt_ctx == nullptr || dec_ctx == nullptr) {
        throw std::runtime_error("Bad init params for filter!");
    }

    const AVFilter *buffersrc = avfilter_get_by_name("buffer");
    const AVFilter *buffersink = avfilter_get_by_name("buffersink");
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs = avfilter_inout_alloc();
    AVBufferSrcParameters *par = av_buffersrc_parameters_alloc();

    AVRational time_base = fmt_ctx->streams[index]->time_base;

    mFilterGraph = avfilter_graph_alloc();
    if (outputs == nullptr || inputs == nullptr || !mFilterGraph) {
        throw std::runtime_error("Alloc filter graph failed!");
    }

    char args[512];
    /* buffer video source: the decoded frames from the decoder will be inserted here. */
    snprintf(args, sizeof(args), "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d", dec_ctx->width,
             dec_ctx->height, dec_ctx->hw_frames_ctx ? AV_PIX_FMT_VAAPI : dec_ctx->pix_fmt, time_base.num,
             time_base.den, dec_ctx->sample_aspect_ratio.num, dec_ctx->sample_aspect_ratio.den);

    ret = avfilter_graph_create_filter(&mBufferSrcCtx, buffersrc, "in", args, NULL, mFilterGraph);
    if (ret < 0) {
        throw std::runtime_error("Cannot create buffer source\n");
    }
    if (dec_ctx->hw_frames_ctx) {
        par->hw_frames_ctx = dec_ctx->hw_frames_ctx;
        ret = av_buffersrc_parameters_set(mBufferSrcCtx, par);
        if (ret < 0)
            throw std::runtime_error("Failed to set buffersrc param\n");
    }
    av_freep(&par);

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

    if (dec_ctx->hw_frames_ctx) {
        for (size_t i = 0; i < mFilterGraph->nb_filters; i++) {
            mFilterGraph->filters[i]->hw_device_ctx = av_buffer_ref(dec_ctx->hw_device_ctx);
        }
    }

    ret = avfilter_graph_config(mFilterGraph, NULL);
    if (ret < 0) {
        throw std::runtime_error("Config filter graph error!\n");
    }

    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);

    mHWPixelFormat = hw_pix_fmt;
    mSWPixelFormat = dec_ctx->sw_pix_fmt;
    mInitialized = true;
    mName = name;
}

void Filter::Start() {
    if (!mInitialized)
        return;

    if (mInputFrameQueue.get() == nullptr || mOutputFrameQueue.get() == nullptr)
        throw std::logic_error("Missing some filter init steps!");

    mRunning = true;
    mWorkingThread = std::thread(&Filter::WorkingFunction, this);
}

void Filter::flush() {
    if (!mRunning)
        return;

    INFO("flushing");
    mInputFrameQueue->wait_until_empty();
    Frame last_frame = {0};
    mInputFrameQueue->push_front(last_frame);
}

void Filter::Stop() noexcept {
    if (!mRunning)
        return;

    if (mWorkingThread.joinable()) {
        flush();
        mRunning = false;
        mWorkingThread.join();
    }
}

void Filter::WorkingFunction() {
    Frame frame_input = {0};
    AVFrame *filter_frame = av_frame_alloc();
    assert(filter_frame);

    for (;;) {
        if (!mRunning)
            break;

        mInputFrameQueue->pop_back(&frame_input);
        AVFrame *input_frame = frame_input.av_frame;

        // empty Frame indicates end of stream
        if (input_frame == nullptr) {
            break;
        }

        VLOG(2) << "filter " << mName << " pop frame: " << input_frame << std::endl;

        if (av_buffersrc_add_frame_flags(mBufferSrcCtx, input_frame, AV_BUFFERSRC_FLAG_KEEP_REF) < 0)
            throw std::runtime_error("Error while feeding the filtergraph\n");

        int ret = 0;
        while (ret >= 0) {
            ret = av_buffersink_get_frame(mBufferSinkCtx, filter_frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                break;
            }

            if (ret < 0) {
                throw std::runtime_error("Error while getting the result \n");
            }

            Frame frame_output = {0};
            AVFrame *output = av_frame_alloc();
            assert(output);
            av_frame_ref(output, filter_frame);
            frame_output.av_frame = output;
            mOutputFrameQueue->push_front(frame_output);
            VLOG(2) << "filter " << mName << " push frame: " << output << std::endl;

            av_frame_unref(filter_frame);
        }

        av_frame_free(&input_frame);
    }

    av_frame_free(&filter_frame);
}

} // namespace deep_player
