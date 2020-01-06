// Copyright (C) 2019 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <gflags/gflags.h>

/// @brief Message for help argument
static const char help_message[] = "Print a usage message";

/// @brief Message for images argument
static const char input_video_message[] = "Required. Path to a video file";

/// @brief Message do not show processed video
static const char no_show_processed_video[] = "Optional. Do not show processed video.";

/// @brief Message filter description
static const char filter_description[] = "Required. First filter description.";

static const char filter_description_optional[] = "Optional. Appended filters description.";

/// @brief Message decoder device name
static const char decoder_device_name[] = "Optional. Hardware device name of video decoder: e.g vaapi";

/// @brief Message for fps argument
static const char fps_output_message[] = "Optional. Maximum FPS for playing video";

/// @brief Message for extra hw frames
static const char extra_hw_frames_message[] = "Optional. Allocate more hw frames for the pipeline";

/// @brief Message for packet queue
static const char demux_queue_message[] = "Required. Set output packet queue size of demuxer";

/// @brief Message for decoder queue
static const char decode_queue_message[] = "Required. Set queue size of decoer output";

/// @brief Message for filter queue
static const char filter_queue_message[] = "Required. Set queue size of filter output";

/// @brief Message for debug log
static const char debug_log_message[] = "Optional. Enable debugging log of ffmpeg libraries";

/// \brief Define flag for showing help message<br>
DEFINE_bool(h, false, help_message);

DEFINE_string(input, "", input_video_message);

DEFINE_string(filter0_desc, "", filter_description);

DEFINE_string(filter1_desc, "", filter_description_optional);

DEFINE_string(filter2_desc, "", filter_description_optional);

DEFINE_string(decode_dev, "", decoder_device_name);

DEFINE_int32(extra_hw_frames, 32, extra_hw_frames_message);

DEFINE_int32(queue_pkt, 4, demux_queue_message);

DEFINE_int32(queue_dec, 4, decode_queue_message);

DEFINE_int32(queue_filter, 2, filter_queue_message);

DEFINE_bool(debug_log, false, debug_log_message);

DEFINE_bool(no_show, false, no_show_processed_video);

DEFINE_double(fps, -1, fps_output_message);

/**
 * \brief This function shows a help message
 */

static void showUsage() {
    std::cout << std::endl;
    std::cout << "deep_play [OPTION]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << std::endl;
    std::cout << "    -h                         " << help_message << std::endl;
    std::cout << "    -input \"<path>\"            " << input_video_message << std::endl;
    std::cout << "    -decode_dev                " << decoder_device_name << std::endl;
    std::cout << "    -extra_hw_frames           " << extra_hw_frames_message << std::endl;
    std::cout << "    -no_show                   " << no_show_processed_video << std::endl;
    std::cout << "    -filter0_desc              " << filter_description << std::endl;
    std::cout << "    -filter1_desc              " << filter_description_optional << std::endl;
    std::cout << "    -filter2_desc              " << filter_description_optional << std::endl;
    std::cout << "    -fps                       " << fps_output_message << std::endl;
    std::cout << "    -queue_pkt                 " << demux_queue_message << std::endl;
    std::cout << "    -queue_dec                 " << decode_queue_message << std::endl;
    std::cout << "    -queue_filter              " << filter_queue_message << std::endl;
}
