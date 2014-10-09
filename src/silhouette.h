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

    int distanceFrom (const cv::Rect &rect) const;
    void addPos(const Rect &newPos);
    void plot(Mat frame);

    bool getUpdated() const;
    void setUpdated(bool value);

    int getGostLife() const;
    void setGostLife(int value);

private:
    Scalar color;

    bool updated;

    list<cv::Rect> previousPos;

    // If the person is present but not detected
    int gostLife;
};

#endif // SILHOUETTE_H
