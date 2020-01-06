/*******************************************************************************
 * Copyright (C) 2018-2019 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "demuxer.h"

namespace deep_player {

Demuxer::Demuxer() {
}

void Demuxer::Init(const InputSource::Ptr &input) {
    mInput = input;
    if (mInput.get() == nullptr)
        throw std::runtime_error("please set input source first!");

    mAVFormatContext = mInput->GetAVFormatContext();
    if (mAVFormatContext == nullptr)
        throw std::runtime_error("AVFormatContext invalid!");

    int ret = av_find_best_stream(mAVFormatContext, AVMEDIA_TYPE_VIDEO, -1, -1, &mAVCodec, 0);
    if (ret < 0)
        throw std::runtime_error("Cannot find video input stream!");

    mVideoStreamIndex = ret;
    mInitialized = true;
}

void Demuxer::SetAVPacketBufferQueue(const bounded_buffer<Packet>::Ptr &packetBufQueue) noexcept {
    this->mPacketQueue = packetBufQueue;
}

void Demuxer::Start() {
    if (mVideoStreamIndex < 0 || mInput.get() == nullptr || mPacketQueue.get() == nullptr)
        throw std::runtime_error("Missing some demuxer init steps!");

    mRunning = true;
    this->mWorkingThread = std::thread(&Demuxer::WorkingFunction, this);
}

void Demuxer::Stop() {
    if (mWorkingThread.joinable()) {
        mRunning = false;
        mWorkingThread.join();
    }

    mInitialized = false;
}

void Demuxer::WorkingFunction() {
    AVPacket *av_packet = av_packet_alloc();
    assert(av_packet);

    for (;;) {
        if (!mRunning)
            break;

        int ret = av_read_frame(mAVFormatContext, av_packet);
        if (ret < 0) {
            std::cout << "dmux ended!" << std::endl;
            mEOSReached = true;
            break;
        }

        if (mVideoStreamIndex != av_packet->stream_index) {
            av_packet_unref(av_packet);
            continue;
        }

        Packet packet = {0};
        AVPacket *output = av_packet_alloc();
        av_packet_ref(output, av_packet);
        packet.av_packet = output;
        mPacketQueue->push_front(packet);

        VLOG(2) << "demux packet pts: " << av_packet->pts;

        av_packet_unref(av_packet);
    }

    av_packet_free(&av_packet);
}

} // namespace deep_player