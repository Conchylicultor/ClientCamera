ClientCamera
============

## Introduction

Client program which retrieve the frame from the camera, detect and track the persons and send the extracted features to the MQTT broker.

## Installation and Setup

After cloning the project, just run the udpade script in order to keep the project up-to-date.

```
chmod +x update.sh
./update.sh
cd build/
./ClientCamera
```

The working directory when launching the program has to be a sub directory.

The data are saved on a subfolder on the same level as the root folder (in order to insure the communication with the other programs)

The structure must look something like this:
* _ReidPath/ClientCamera/_ : Record the pedestrians
* _ReidPath/ClientCamera/build/_ : Contain the executable file
* _ReidPath/ClientOnlineCamera/_ : Other program which will have access to the data folder
* _ReidPath/Data/_ : Folder were data are saved, must contain the subfolder _Traces_ where the extracted sequences will be saved

Moreover the folder _ClientCamera_ must contain a video.yml indicating the url of the camera or files that have to be loaded.

Here is is an example of video.yml:

```
%YAML:1.0
videoNames:
    - '/home/user/Data/Recordings/00_Vid_1.mp4'
    - '/home/user/Data/Recordings/00_Vid_2.mp4'
    - 'http://login:password@192.168.0.168/axis-cgi/mjpg/video.cgi?;type=.mjpg'
recordingVid: 0
recordingTrace: 1
hideGui: 0
lifeTime: 300
```

Use the hideGui option for the release version (record in background without showing anything). lifeTime represent the number of minute before the program automatically shutdown.

## Output

Each person detected is recorded if it has been tracked enough time (at least X frames). The list of all the recorded sequences is contain in the _Traces/_ folder on the file _traces.txt_.

Each sequence is recorded as XX_YY_ZZ.

## Calibration

There is an optional calibration to track the path of the differents sequences. Here is how to use it. It require an additionnal subfolder _calibration/_ on the same level that the _src/_ or the _build/_ directories.
This folder has to contain the following images:
* *map.png*: a map of the evironement where the track will be traced
* *vidX.png* (X being the camera number starting with 1, for instance *vid3.png*): A screenshot of the camera in black & white containing the 4 calibration points for the homography.
* *vidX_map.png* (for instance *vid3_map.png*): a black & white map copy of _map.png_ which contains the 4 calibration points for the homography.

The program will automatically do the correspondance between the position inside the camera and position on the map thanks to the calibration points. The result is saved (if recoding enabled) on the _Traces_ file as _map.png_. The 4 calibration points have to be exactly *one* pixel of those color: Red (255,0,0), Magenta (255,0,255), Yellow (255,255,0), green (0,255,0). Of course equivalent position on the map and on the camera have to be of the same color.
