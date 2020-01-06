/*******************************************************************************
 * Copyright (C) 2018-2019 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "decoder.h"

#ifdef USE_OPENCV
#include "opencv_utils.h"
#endif

namespace deep_player {

static enum AVPixelFormat get_hw_format(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts) {
    const enum AVPixelFormat *p;
    Decoder *decoder = reinterpret_cast<Decoder *>(ctx->opaque);
    const enum AVPixelFormat hw_pix_fmt = decoder->GetHWPixelFormat();

    for (p = pix_fmts; *p != -1; p++) {
        if (*p == hw_pix_fmt)
            return *p;
    }

    fprintf(stderr, "Failed to get HW surface format.\n");
    return AV_PIX_FMT_NONE;
}

Decoder::Decoder()
    : mInitialized(false), mAVCodecContext(nullptr), mHWDeviceCtx(nullptr), mHWPixelFormat(AV_PIX_FMT_NONE) {
}

Decoder::~Decoder() {
    avcodec_free_context(&mAVCodecContext);
    av_buffer_unref(&mHWDeviceCtx);
    mInitialized = false;
}

void Decoder::SetAVPacketBufferQueue(const bounded_buffer<Packet>::Ptr &packetBufQueue) noexcept {
    this->mPacketQueue = packetBufQueue;
}

void Decoder::SetAVFrameBufferQueue(const bounded_buffer<Frame>::Ptr &frameBufQueue) noexcept {
    this->mFrameQueue = frameBufQueue;
}

void Decoder::Init(const Demuxer::Ptr &demuxer, const AVCodec *codec, const char *deviceName, int extra_frames) {
    if (demuxer.get() == nullptr || codec == nullptr)
        throw std::runtime_error("critical parameters are null!");

    if (!(mAVCodecContext = avcodec_alloc_context3(codec)))
        throw std::runtime_error("allocate codec context failed!");

    const AVCodecParameters *codecParam = demuxer->GetVideoCodecParameters();
    if (codecParam == nullptr || avcodec_parameters_to_context(mAVCodecContext, codecParam) < 0)
        throw std::runtime_error("convert codec parameters to context failed!");

    std::string device = deviceName;
    if (deviceName != nullptr && !device.empty())
        init_hw_context(codec, deviceName, extra_frames);

    if (avcodec_open2(mAVCodecContext, codec, NULL) < 0)
        throw std::runtime_error("Failed to open codec for stream.\n");

    mDemux = demuxer;
    mInitialized = true;
}

void Decoder::Start() {
    if (!mInitialized || mPacketQueue.get() == nullptr)
        throw std::runtime_error("Missing some decoder init steps!");

    this->mWorkingThread = std::thread(&Decoder::WorkingFunction, this);
}

void Decoder::Stop() noexcept {
    if (!mInitialized)
        return;

    if (mWorkingThread.joinable()) {
        Packet empty_packet = {0};
        AVPacket *pkt = av_packet_alloc();
        pkt->size = 0;
        pkt->data = NULL;
        empty_packet.av_packet = pkt;
        mPacketQueue->push_front(empty_packet);
        mWorkingThread.join();
    }

    std::cout << "decoded: " << mAVCodecContext->frame_number << " frames." << std::endl;
}

bool Decoder::init_hw_context(const AVCodec *decoder, const char *name, int extra_frames) {
    AVHWDeviceType type = av_hwdevice_find_type_by_name(name);
    if (type == AV_HWDEVICE_TYPE_NONE) {
        fprintf(stderr, "Device type %s is not supported.\n", name);
        return false;
    }

    for (int i = 0;; i++) {
        const AVCodecHWConfig *config = avcodec_get_hw_config(decoder, i);
        if (config == nullptr) {
            fprintf(stderr, "Decoder %s does not support device type %s.\n", decoder->name,
                    av_hwdevice_get_type_name(type));
            return false;
        }
        if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX && config->device_type == type) {
            mHWPixelFormat = config->pix_fmt;
            break;
        }
    }

    mAVCodecContext->opaque = this;
    mAVCodecContext->get_format = get_hw_format;
    mAVCodecContext->thread_count = 1; // FIXME: vaapi decoder multi thread issue
    mAVCodecContext->extra_hw_frames = extra_frames;
    mAVCodecContext->hwaccel_flags |= AV_HWACCEL_FLAG_ALLOW_PROFILE_MISMATCH;

    if ((av_hwdevice_ctx_create(&mHWDeviceCtx, type, NULL, NULL, 0)) < 0) {
        fprintf(stderr, "Failed to create specified HW device.\n");
        return false;
    }
    mAVCodecContext->hw_device_ctx = av_buffer_ref(mHWDeviceCtx);
    VLOG(1) << "allocate hw device ctx:" << mAVCodecContext->hw_device_ctx << std::endl;

    return true;
}

int Decoder::decode(AVFrame *frame) {
    while (1) {
        int ret = avcodec_receive_frame(mAVCodecContext, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return 0;

        if (ret < 0)
            return -1;

        AVFrame *output = av_frame_alloc();
        av_frame_ref(output, frame);
        Frame frame_out = {0};
        frame_out.av_frame = output;
        mFrameQueue->push_front(frame_out);
        VLOG(2) << "Decoder push frame: " << output << std::endl;
        av_frame_unref(frame);
    }

    return 0;
}

void Decoder::WorkingFunction() {
    Packet packet = {0};

    AVFrame *decoded_frame = av_frame_alloc();
    assert(decoded_frame);

    for (;;) {
        mPacketQueue->pop_back(&packet);
        AVPacket *input_packet = packet.av_packet;

        // exit when receive an empty packet
        if (input_packet == nullptr)
            break;

        VLOG(2) << "debug ptk: " << input_packet->pts << std::endl;

        int ret = avcodec_send_packet(mAVCodecContext, input_packet);
        if (ret < 0) {
            throw std::runtime_error("Send packet error!");
        }

        if (decode(decoded_frame) < 0) {
            av_packet_free(&input_packet);
            throw std::runtime_error("Error while decoding!");
        }

        if (input_packet->size == 0)
            break;
        av_packet_free(&input_packet);
    }

    av_frame_free(&decoded_frame);
}

} // namespace deep_player
