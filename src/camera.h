#ifndef CAMERA_H
#define CAMERA_H

#include <iostream>
#include "opencv2/opencv.hpp"

#include "silhouette.h"

#define REID_WIDTH 640
#define REID_HEIGHT 480

class Camera
{
public:
    Camera(std::string pathVid, int clientId, bool record = false, bool hideGui = false);
    ~Camera();

    void grab();
    void play();

    void togglePause();

private:
    static int nbCams;
    std::string nameVid;

    cv::VideoCapture cap;
    cv::VideoWriter writer;

    bool recording;
    bool hidingGui;

    bool success;
    bool pause;

    cv::Mat frame;

    // Spacial localisation
    bool spacialLocalisation;
    cv::Mat homographyMatrix;

    void loadTransformationMatrix();

    // ----- Pipeline -----

    // Step1: Person detection
    void detectPersons();
    cv::BackgroundSubtractorMOG2 backgroundSubstractor;
    cv::Mat fgMask;
    std::vector<cv::Rect> personsFound;

    // Step2: Tracking
    void tracking();
    std::list<Silhouette*> listCurrentSilhouette;

    // Step3: Features extraction
    void computeFeatures();

    void addVisualInfos();
};

#endif // CAMERA_H
