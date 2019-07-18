#!/bin/bash

./object_detection_yolo_host --image=bird.jpg 2>&1 | tee yolobird.log
./object_detection_yolo --video=run.mp4 2>&1 | tee yolorun.log

