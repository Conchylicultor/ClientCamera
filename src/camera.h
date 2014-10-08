#ifndef CAMERA_H
#define CAMERA_H

#include <iostream>
#include "opencv2/opencv.hpp"

#define REID_WIDTH 640
#define REID_HEIGHT 480

using namespace std;
using namespace cv;

class Camera
{
public:
    Camera(string pathVid, bool record = false);
    ~Camera();

    void grab();
    void play();

private:
    static int nbCams;
    string nameVid;

    VideoCapture cap;
    VideoWriter writer;

    bool recording;

    bool success;

    Mat frame;

    // Pipeline

    // Step1: Person detection
    void detectPersons();
    vector<Rect> personsFound;

    void addVisualInfos();
};

#endif // CAMERA_H
