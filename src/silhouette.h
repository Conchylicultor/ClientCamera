#ifndef SILHOUETTE_H
#define SILHOUETTE_H

#include <iostream>
#include <chrono>
#include <list>
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

class Silhouette
{
public:
    Silhouette();

    static void setRecordTrace(bool value);

    int distanceFrom(const cv::Rect &rect) const;
    void addPos(const Rect &newPos);
    void plot(Mat &frame);
    void addFrame(Mat &frame, Mat &fgMask); // And save eventually
    void saveCamInfos(string nameVid);

    bool getUpdated() const;
    void setUpdated(bool value);

    int getGostLife() const;
    void setGostLife(int value);

private:
    static bool recordTrace;// If extracted pictures are savedon disk or not

    static int nbIds; // Total number of silhouette
    int id; // Id of the current silhouette

    Scalar color; // Show on the video

    list<cv::Rect> previousPos;

    // ----- For tracking -----

    // Tell if the silhouette is visible in the current frame
    bool updated;
    // If the person is present but not detected
    int gostLife;

    // ----- For save -----
    list< pair<Mat, Mat> > extFrames;
    chrono::system_clock::time_point beginTime;
};

#endif // SILHOUETTE_H
