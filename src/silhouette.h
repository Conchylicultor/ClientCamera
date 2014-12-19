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

    static void setRecordTrace(bool value);

    int distanceFrom(const cv::Rect &rect) const;
    void addPos(const Rect &newPos);
    void plot(Mat &frame);
    void updateFeatures(Mat &frame, Mat &fgMask);

    bool getUpdated() const;
    void setUpdated(bool value);

    int getGostLife() const;
    void setGostLife(int value);

private:
    static int recordTrace;// If extracted pictures are savedon disk or not

    static int nbIds; // Total number of silhouette
    int id; // Id of the current silhouette

    Scalar color;

    list<cv::Rect> previousPos;

    // ----- For tracking -----

    // Tell if the silhouette is visible in the current frame
    bool updated;
    // If the person is present but not detected
    int gostLife;

    // ----- For save -----
    list< pair<Mat, Mat> > extFrames;

    // ----- For feature extraction -----

    // Color histogram
    void histRGB(Mat &frame, Mat &fgMask);
    Mat hist1;
    Mat hist2;
    Mat hist3;
};

#endif // SILHOUETTE_H
