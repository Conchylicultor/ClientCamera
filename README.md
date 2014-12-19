ClientCamera
============

Client program which retrieve the frame from the camera, detect and track the persons and send the extracted features to the MQTT broker.

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
* ReidPath/ClientCamera/ : Record the pedestrians
* ReidPath/ClientCamera/build/ : Contain the executable file
* ReidPath/ClientOnlineCamera/ : Other program which will have access to the data folder
* ReidPath/Data/ : Folder were data are saved

Moreover the folder ClientCamera must contain a video.yml indicating the url of the camera or files that have to be loaded.

Here is is an example of video.yml:

```
%YAML:1.0
videoNames:
    - '/home/user/Data/Recordings/00_Vid_1.mp4'
    - '/home/user/Data/Recordings/00_Vid_2.mp4'
    - 'http://login:password@192.168.0.168/axis-cgi/mjpg/video.cgi?;type=.mjpg'
recordingVid: 0
recordingTrace: 1
```
