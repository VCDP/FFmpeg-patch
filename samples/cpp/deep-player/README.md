# deep-player
* This is a sample to use ffmpeg API based on the libraries with DNN based analytics plugins.  
  It can support VAAPI based hardware decoder and Software decoder, 3 filter graphs at most.  
  The analytics metadata can be overlayed on top of the video.

* To try this sample, please install below dependencies:
    - install OpenCV and some other dependencies by script `install_dependencies.sh`
    - install ffmpeg with video analytics plugin as shared libraries

* After install dependencies, you can build it:
```sh
    mkdir -p build && cd build
    cmake .. && make -j`nproc`
```

* Here is one example to use the deep_play sample:
```sh

MODEL1="face-detection-adas-0001.xml"
MODEL2="emotions-recognition-retail-0003.xml"
MODEL3="age-gender-recognition-retail-0013.xml"

MODEL1_PROC="face-detection-adas-0001.json"
MODEL2_PROC="emotions-recognition-retail-0003.json"
MODEL3_PROC="age-gender-recognition-retail-0013.json"

./sample/deep_play -input ~/workspace/tests/0.h264 \
    -filter0_desc "detect=model=$MODEL1:model_proc=$MODEL1_PROC:device=CPU:nireq=4" \
    -filter1_desc "classify=model=$MODEL2:model_proc=$MODEL2_PROC:device=CPU:nireq=2" \
    -filter2_desc "classify=model=$MODEL3:model_proc=$MODEL3_PROC:device=CPU:nireq=2" \
    -queue_filter=6 -extra_hw_frames=64\
    -decode_dev="vaapi" -debug_log=false -no_show=false
```
Press `ESC` to stop the playback.

* Detailed usage:

```sh
./sample/deep_play -h

deep_play [OPTION]
Options:

    -h                         Print a usage message
    -input "<path>"            Required. Path to a video file
    -decode_dev                Optional. Hardware device name of video decoder: e.g vaapi
    -extra_hw_frames           Optional. Allocate more hw frames for the pipeline
    -no_show                   Optional. Do not show processed video.
    -filter0_desc              Required. First filter description.
    -filter1_desc              Optional. Appended filters description.
    -filter2_desc              Optional. Appended filters description.
    -fps                       Optional. Maximum FPS for playing video
    -queue_pkt                 Required. Set output packet queue size of demuxer
    -queue_dec                 Required. Set queue size of decoer output
    -queue_filter              Required. Set queue size of filter output
```