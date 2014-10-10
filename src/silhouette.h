#ifndef SILHOUETTE_H
#define SILHOUETTE_H

#include <iostream>
#include <list>
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

class Silhouette
{
public:
    Silhouette();

    int distanceFrom(const cv::Rect &rect) const;
    void addPos(const Rect &newPos);
    void plot(Mat &frame);
    void updateFeatures(Mat &frame, Mat &fgMask);

    bool getUpdated() const;
    void setUpdated(bool value);

    int getGostLife() const;
    void setGostLife(int value);

private:
    Scalar color;

    list<cv::Rect> previousPos;

    // ----- For tracking -----

    // Tell if the silhouette is visible in the current frame
    bool updated;
    // If the person is present but not detected
    int gostLife;

    // ----- For feature extraction -----
};

#endif // SILHOUETTE_H
