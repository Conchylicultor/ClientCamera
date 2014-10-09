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

    bool getUpdated() const;
    void setUpdated(bool value);

    int distanceFrom (const cv::Rect &rect) const;
    void addPos(const Rect &newPos);

private:
    bool updated;

    list<cv::Rect> previousPos;
};

#endif // SILHOUETTE_H
