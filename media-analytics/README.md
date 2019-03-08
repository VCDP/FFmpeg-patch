# Media analytics patches
- Patches to enable Intel inference engine as a DNN backend in FFmpeg(based on version 4.1)

## Building steps
- Build Intel DLDT inference engine from source code, reference steps [link](https://github.com/linxie47/dldt/tree/build_script/build-script)
- Clone FFmpeg source code and checkout to `release/4.1`
- Apply the patches in this repo
- Follow official compiling guide to build ffmpeg [link](https://trac.ffmpeg.org/wiki/CompilationGuide)

## Command line examples
```sh
    Face Detection:
    ffmpeg -i ~/Videos/xxx.mp4 -vf detect=model=./face-detection-adas-0001/FP32/face-detection-adas-0001.xml:name=face -an -f null /dev/null
    ffmpeg -i ~/Videos/xxx.mp4 -vf detect=model=./face-detection-retail-0004/FP32/face-detection-retail-0004.xml:name=face -an -f null /dev/null

    Object Detection(w/ labels):
    ffmpeg -i ~/Videos/xxx.mp4 -vf detect=model=./mobilenet-ssd.xml:label=./object_labels.txt:name=objects -an -f null /dev/null

    Face + Emotion (please create a label file and edit its content as: neutral,happy,sad,surprise,anger)
    ffmpeg -i ~/Videos/xxx.mp4 -vf detect=model=./face-detection-adas-0001/FP32/face-detection-adas-0001.xml:name=face, \
    classify=model=./emotions_recognition/emotions-recognition-retail-0003.xml:label=./emotions_recognition/emotion-labels.txt:names=emotion \
    -an -f null /dev/null

    Face + Emotion + Age-Gender (please also create a label file and edit its content as: female,male)
    ffmpeg -i ~/Videos/xxx.mp4 -vf "detect=model=./face-detection-retail-0004/FP32/face-detection-retail-0004.xml:name=face, \
    classify=model=./age_gender/age-gender-recognition-retail-0013.xml&./emotions_recognition/emotions-recognition-retail-0003.xml: \
    label=./age_gender/gender-labels.txt&./emotions_recognition/emotion-labels.txt:names=age-gender&emotion" \
    -an -f null /dev/null

    Face + Reidentification
    ffmpeg ...... \
    classify=model=./fae-reidentification-retail-0095.xml:label=./face-labels.txt:name=face_id:feature_file=./registered_faces.bin -an -f null /dev/null

    To write IE metadata to json format:
    ffmpeg ...... -f iemetadata xxx.json
    ffmpeg ...... -f iemetadata -source <url> -custom_tag "key1:value1,key2:value2..." xxx.json

    To write IE metadata and send kafka message:
    ffmpeg ...... -f iemetadata kafka://<hostname:port>/<topic>
```

