#include "silhouette.h"

#define RECORD_TRACE 0

int Silhouette::nbIds = 0;

Silhouette::Silhouette() :
    updated(false),
    gostLife(-1)
{
    nbIds++;
    id = nbIds;

    // Choose a random color for the person
    color[0] = std::rand() % 255;
    color[1] = std::rand() % 255;
    color[2] = std::rand() % 255;
}

int Silhouette::distanceFrom(const Rect &rect) const
{
    // Centers of the rectangles
    cv::Point c1(     rect.x          +        rect.width/2       ,
                      rect.y          +        rect.height/2      );
    cv::Point c2(previousPos.back().x + previousPos.back().width/2,
                 previousPos.back().y + previousPos.back().height/2);

    return (int)sqrt((c2.x - c1.x)*(c2.x - c1.x) + (c2.y - c1.y)*(c2.y - c1.y));
}

void Silhouette::addPos(const Rect &newPos)
{
    previousPos.push_back(newPos);

    // Delete first if list too long
    if(previousPos.size() > 300)
    {
        cout << "Warning: Silhouette reach big size !" << endl;
    }
}

void Silhouette::plot(Mat &frame)
{
    rectangle(frame, previousPos.back(), color, 2);
}

void Silhouette::updateFeatures(Mat &frame, Mat &fgMask)
{
    // TODO: Other conditions to extract the features ?
    Mat persImg  = frame (previousPos.back());
    Mat persMask = fgMask(previousPos.back());

    imshow("persImg", persImg);
    imshow("persMask", persMask);

    // Save on disk >> For the database
    extFrames.push_back(pair<Mat, Mat>(persImg, persMask));

    // We only save if the person has been recorded more than 10 times (frames)
    if(RECORD_TRACE)
    {
        const int minToSave = 10;
        if(extFrames.size() > minToSave)
        {
            // Filter: aspect ratio
            if(extFrames.back().first.rows / extFrames.back().first.cols > 1.4)
            {
                imwrite("/home/etienne/__A__/Data/Traces/" + std::to_string(id) + "_" + std::to_string(extFrames.size()) + ".png", extFrames.back().first);
                imwrite("/home/etienne/__A__/Data/Traces/" + std::to_string(id) + "_" + std::to_string(extFrames.size()) + "_mask.png", extFrames.back().second);
            }
        }
    }
}

bool Silhouette::getUpdated() const
{
    return updated;
}

void Silhouette::setUpdated(bool value)
{
    updated = value;
}

int Silhouette::getGostLife() const
{
    return gostLife;
}

void Silhouette::setGostLife(int value)
{
    gostLife = value;
}

