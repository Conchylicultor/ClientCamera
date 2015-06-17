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

The data are saved on a subfolder on the same level as the root folder (in order to insure the communication with the other programs). The path structure must look like this:
* _ReidPath/ClientCamera/_ : This repository (program which record the pedestrians)
* _ReidPath/ClientCamera/build/_ : Contain the executable file (**has to be the working directory when running the program !**)
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
trackingHog: 1
recordingVid: 0
recordingTrace: 1
hideGui: 0
lifeTime: 800
```

Use the _hideGui_ option for the release version (record in background without showing anything). _lifeTime_ represent the number of minute before the program automatically shutdown. _trackingHog_ indicate to the program if it has to use HoG for detecting the pedestrian instead of a simple background detection. _recordingVid_ simply record the video as it is showed at the screen.

**Warning:** If _recordingTrace_ is on, all the previous recorded content inside the _Traces/_ directory will be removed when the program is launch and replaced by the new recording.

## Output

Each person detected is recorded if it has been tracked enough time (at least X frames). The list of all the recorded sequences is contain in the _Traces/_ folder on the file _traces.txt_.

Each sequence recorded is defined by an id of the form *XX_YY* where _XX_ is the client number id (found in the _ClientOnlineCamera/_ directory) and _YY_ is the number of sequence for this particular client. That mean that each sequence is guaranteed to have a unique id even if the camera are divided among multiple clients. The sequence contain the following elements:
* *XX_YY_list.txt* : Contains name list of the different image extracted.
* *XX_YY_ZZ.png* and *XX_YY_ZZ_mask.png* : The extracted image of the people (and the corresponding mask images).
* *XX_YY_cam.yml* : Contains informations about the time and the camera where the person has been detected as well as the entrance and exit directions of the person on the camera.
* *XX_YY_pos.txt* : If the homography is activated (see section calibration below) contains the list of the global map positions.

You can manually label the sequences (in order to train or evaluate the reidentification algorithm) by adding a label(without space!) to the sequence id on the output _trace.txt_ file. Here for example:

```
----- 0_1 -----
0_1_cam
0_1_pos
----- 0_9 -----
0_9_cam
0_9_pos
----- 0_11 -----
0_11_cam
0_11_pos
...
```
Could become:
```
----- 0_1:Sophia -----
0_1_cam
0_1_pos
----- 0_9:Kotono -----
0_9_cam
0_9_pos
----- 0_11:Sophia -----
0_11_cam
0_11_pos
...
```

The labelization process could be assisted assisted by the computer by using the NetworkVisualizer.

## Calibration

There is an optional calibration to track the path of the differents sequences. Here is how to use it. It require an additionnal subfolder _calibration/_ on the same level that the _src/_ or the _build/_ directories.
This folder has to contain the following images:
* *map.png*: a map of the evironement where the track will be traced
* *vidX.png* (X being the camera number starting with 1, for instance *vid3.png*): A screenshot of the camera in black & white containing the 4 calibration points for the homography.
* *vidX_map.png* (for instance *vid3_map.png*): a black & white map copy of *map.png* which contains the 4 calibration points for the homography.

The program will automatically do the correspondance between the position inside the camera and position on the map thanks to the calibration points. The result is saved (if recoding enabled) on the _Traces_ file as _map.png_. The 4 calibration points have to be exactly **one** pixel of those color: Red (255,0,0), Magenta (255,0,255), Yellow (255,255,0), green (0,255,0). Of course equivalent position on the map and on the camera have to be of the same color.
