#ifndef CAMERA_H
#define CAMERA_H

#include <iostream>
#include "opencv2/opencv.hpp"

#include "silhouette.h"

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

    void togglePause();

private:
    static int nbCams;
    string nameVid;

    VideoCapture cap;
    VideoWriter writer;

    bool recording;

    bool success;
    bool pause;

    Mat frame;

    // ----- Pipeline -----

    // Step1: Person detection
    void detectPersons();
    BackgroundSubtractorMOG2 backgroundSubstractor;
    Mat fgMask;
    vector<Rect> personsFound;

    // Step2: Tracking
    void tracking();
    list<Silhouette*> listCurrentSilhouette;

    void addVisualInfos();
};

#endif // CAMERA_H
