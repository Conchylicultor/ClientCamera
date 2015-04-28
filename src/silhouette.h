#ifndef SILHOUETTE_H
#define SILHOUETTE_H

#include <iostream>
#include <chrono>
#include <vector>
#include "opencv2/opencv.hpp"

class Silhouette
{
public:
    Silhouette();

    static void setRecordTrace(bool value);
    static void setClientId(int value);

    int distanceFrom(const cv::Rect &rect) const;
    void addPos(cv::Rect &newPos);
    void plot(cv::Mat &frame);
    void addFrame(cv::Mat &frame, cv::Mat &fgMask, bool useHomographyMatrix); // And save eventually
    void saveCamInfos(std::string nameVid, const cv::Mat &homographyMatrix);

    bool getUpdated() const;
    void setUpdated(bool value);

    int getGostLife() const;
    void setGostLife(int value);

private:
    static bool recordTrace;// If extracted pictures are saved on disk or not
    static int clientId;// Used into the filename (when recording)

    static int nbIds; // Total number of silhouette
    int id; // Id of the current silhouette

    cv::Scalar color; // Show on the video

    std::vector<cv::Rect> previousPos;

    // ----- For tracking -----

    // Tell if the silhouette is visible in the current frame
    bool updated;
    // If the person is present but not detected
    int gostLife;

    // ----- For save -----
    std::list< std::pair<cv::Mat, cv::Mat> > extFrames;
    std::chrono::system_clock::time_point beginTime;
};

#endif // SILHOUETTE_H
