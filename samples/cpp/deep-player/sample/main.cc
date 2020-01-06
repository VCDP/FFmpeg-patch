// Copyright (C) 2019 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <fstream>
#include <glog/logging.h>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unistd.h>
#include <utility>
#include <vector>

#include "decoder.h"
#include "demuxer.h"
#include "filter.h"
#include "input_source.h"
#include "opencv_utils.h"
#include "render.h"

#include "flags.h"

extern "C" {
#include "libavutil/log.h"
}

using namespace deep_player;

static bool ParseAndCheckCommandLine(int argc, char *argv[]) {
    gflags::ParseCommandLineNonHelpFlags(&argc, &argv, true);
    if (FLAGS_h) {
        showUsage();
        return false;
    }

    std::cout << "Parsing input parameters" << std::endl;

    if (FLAGS_input.empty()) {
        throw std::logic_error("Parameter -input is not set");
    }

    if (FLAGS_filter0_desc.empty()) {
        throw std::logic_error("Parameter -filter0_desc is not set");
    }

    return true;
}

/**
 * @brief The entry point of the Inference Engine sample application
 */
int main(int argc, char *argv[]) {
    // Initialize Google's logging library.
    google::InitGoogleLogging(argv[0]);

    // Parsing and validatiing input arguments
    if (!ParseAndCheckCommandLine(argc, argv))
        return 0;

    LOG(INFO) << "Start " << argv[0] << " running";

    if (FLAGS_debug_log)
        av_log_set_level(AV_LOG_DEBUG);

    InputSource::Ptr input = std::make_shared<InputSource>(FLAGS_input);

    Demuxer::Ptr demux = std::make_shared<Demuxer>();
    Decoder::Ptr decode = std::make_shared<Decoder>();
    Filter::Ptr filter0 = std::make_shared<Filter>();
    Render::Ptr render = std::make_shared<Render>();

    demux->Init(input);

    // Set demux packet queue
    bounded_buffer<Packet>::Ptr pkt_q = std::make_shared<bounded_buffer<Packet>>(FLAGS_queue_pkt);
    demux->SetAVPacketBufferQueue(pkt_q);
    decode->SetAVPacketBufferQueue(pkt_q);

    // Set decode avframe queue
    bounded_buffer<Frame>::Ptr dec_to_filter_q = std::make_shared<bounded_buffer<Frame>>(FLAGS_queue_dec);
    decode->SetAVFrameBufferQueue(dec_to_filter_q);

    decode->Init(demux, demux->GetAVCodec(), FLAGS_decode_dev.data(), FLAGS_extra_hw_frames);

    bounded_buffer<Frame>::Ptr filter0_out_q = std::make_shared<bounded_buffer<Frame>>(FLAGS_queue_filter);
    filter0->SetInputAVFrameBufferQueue(dec_to_filter_q);
    filter0->SetOutputAVFrameBufferQueue(filter0_out_q);

    decode->Start();
    demux->Start();

    AVCodecContext *codec_ctx = decode->GetAVCodecContext();
    if (codec_ctx->hw_device_ctx != nullptr) {
        while (codec_ctx->hw_frames_ctx == nullptr) {
            usleep(100);
        }
    }

    LOG(INFO) << "Start filter init.";
    filter0->Init(input->GetAVFormatContext(), demux->GetVideoStreamIndex(), decode->GetAVCodecContext(),
                  decode->GetHWPixelFormat(), FLAGS_filter0_desc.data(), "filter0");

    bounded_buffer<Frame>::Ptr render_q = filter0_out_q;

    Filter::Ptr filter1 = std::make_shared<Filter>();
    if (FLAGS_filter1_desc != "") {
        filter1->Init(input->GetAVFormatContext(), demux->GetVideoStreamIndex(), decode->GetAVCodecContext(),
                      decode->GetHWPixelFormat(), FLAGS_filter1_desc.data(), "filter1");

        bounded_buffer<Frame>::Ptr filter1_out_q = std::make_shared<bounded_buffer<Frame>>(FLAGS_queue_filter);
        filter1->SetInputAVFrameBufferQueue(render_q);
        filter1->SetOutputAVFrameBufferQueue(filter1_out_q);
        render_q = filter1_out_q;
    }

    Filter::Ptr filter2 = std::make_shared<Filter>();
    if (FLAGS_filter2_desc != "") {
        filter2->Init(input->GetAVFormatContext(), demux->GetVideoStreamIndex(), decode->GetAVCodecContext(),
                      decode->GetHWPixelFormat(), FLAGS_filter2_desc.data(), "filter2");

        bounded_buffer<Frame>::Ptr filter2_out_q = std::make_shared<bounded_buffer<Frame>>(FLAGS_queue_filter);
        filter2->SetInputAVFrameBufferQueue(render_q);
        filter2->SetOutputAVFrameBufferQueue(filter2_out_q);
        render_q = filter2_out_q;
    }

    render->Init(input->GetAVFormatContext(), demux->GetVideoStreamIndex(), render_q, FLAGS_no_show);
    render->Start();

    filter0->Start();
    filter1->Start();
    filter2->Start();

    bool lastFrame = false;
    while (!lastFrame) {
        sleep(1);
        if (render->IsEscKey())
            break;

        lastFrame = demux->EOS();
    }

    LOG(INFO) << "Stop demux." << std::endl;
    demux->Stop();

    LOG(INFO) << "Stop decode." << std::endl;
    decode->Stop();

    LOG(INFO) << "Stop filters." << std::endl;
    filter0->Stop();
    filter1->Stop();
    filter2->Stop();

    LOG(INFO) << "Stop render." << std::endl;
    render->Stop();

    LOG(INFO) << "test done." << std::endl;
    return EXIT_SUCCESS;
}
