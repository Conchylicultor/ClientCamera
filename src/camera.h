#ifndef CAMERA_H
#define CAMERA_H

#include <iostream>
#include "opencv2/opencv.hpp"
#include "opencv2/ocl/ocl.hpp"

#include "silhouette.h"

#define REID_WIDTH 640
#define REID_HEIGHT 480

class Camera
{
public:
    Camera(std::string pathVid, int clientId, bool modeTrackingHog, bool record = false, bool hideGui = false);
    ~Camera();

    void grab();
    void play();

    void togglePause();

    // For recording the video of the map traces
    static void initMapTraces(); // Load recording
    static void updateMapTraces(); // Record
    static void closeMapTraces(); // End recording

private:
    static int nbCams;
    std::string nameVid;

    cv::VideoCapture cap;
    cv::VideoWriter writer;

    bool hogMode;
    bool recording;
    bool hidingGui;

    bool success;
    bool pause;

    cv::Mat frame;

    // Spacial localisation
    bool spacialLocalisation;
    cv::Mat homographyMatrix;

    void loadTransformationMatrix();

    // For recording the video of the map traces
    static std::list<MapDot> mapDotList;
    static cv::Mat mapBackground;
    static cv::VideoWriter mapTracesWriter;

    // ----- Pipeline -----

    // Step1: Person detection
    void detectPersons();
    void mergeCloseRoi(std::list<cv::Rect> &roiList, cv::Rect &newRect, int margin) const;
    void extractPersons(const cv::Mat &roi, const cv::Point &roiPos); // Add to the list of persons found the detected persons inside the roi
    static cv::ocl::HOGDescriptor *personDescriptor; // Pointer to avoid unecessary loading if ocl not used
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
