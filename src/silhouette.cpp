#include "silhouette.h"

#include <iostream>
#include <fstream>

#define RECORD_TRACE 0

#define HIST_SIZE 100

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
    // (?) Other conditions to extract the features ?
    Mat persImg  = frame (previousPos.back());
    Mat persMask = fgMask(previousPos.back());

    imshow("persImg", persImg);
    imshow("persMask", persMask);

    // Extract features

    // TODO: Segmentation

    // RGB histogram
    histRGB(frame, fgMask);

    // LBP (Local binary patern): Texture information

    // Save on disk (for built the database)
    if(RECORD_TRACE)
    {
        // Save on disk >> For the database
        extFrames.push_back(pair<Mat, Mat>(persImg, persMask));

        // We only save if the person has been recorded more than 10 times (frames)
        const int minToSave = 10;
        if(extFrames.size() > minToSave)
        {
            // Filter: aspect ratio
            if(extFrames.back().first.rows / extFrames.back().first.cols > 1.4)
            {
                // Compute image id
                string imageId = std::to_string(id) + "_" + std::to_string(extFrames.size());

                // Save image
                imwrite("/home/etienne/__A__/Data/Traces/" + imageId + ".png", extFrames.back().first);
                imwrite("/home/etienne/__A__/Data/Traces/" + imageId + "_mask.png", extFrames.back().second);

                list<string> contentTraces;

                // Write into file
                ifstream fileTracesIn;
                fileTracesIn.open ("/home/etienne/__A__/Data/Traces/traces.txt");
                if(fileTracesIn.is_open())
                {
                    // Read entire file
                    for(string line; std::getline(fileTracesIn, line); )
                    {
                        contentTraces.push_back(line);
                    }
                    fileTracesIn.close();
                }

                // Add content: insert lines
                string titleId = "----- " + std::to_string(id) + " -----";
                list<string>::iterator iter = std::find(contentTraces.begin(), contentTraces.end(), titleId);

                if(iter == contentTraces.end())
                {
                    contentTraces.push_back(titleId);
                    contentTraces.push_back(imageId);
                }
                else
                {
                    iter++;
                    contentTraces.insert(iter, imageId);
                }

                // Write file
                ofstream fileTracesOut;
                fileTracesOut.open ("/home/etienne/__A__/Data/Traces/traces.txt");
                if(!fileTracesOut.is_open())
                {
                    cout << "Error: No file trace out" << endl;
                }
                for(string line : contentTraces)
                {
                    fileTracesOut << line << endl;
                }
                fileTracesOut.close();
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

void Silhouette::histRGB(Mat &frame, Mat &fgMask)
{
    // Conversion to the right color space ???

    // Size of the histogram
    int histSize = HIST_SIZE; // bin size
    float range[] = {0, 256}; // min max values
    const float *ranges[] = {range};

    // Extraction of the histograms
    std::vector<cv::Mat> sourceChannels;
    cv::split(frame, sourceChannels);
    cv::calcHist(&sourceChannels[0], 1, 0, fgMask, hist1, 1, &histSize, ranges, true, false );
    cv::calcHist(&sourceChannels[1], 1, 0, fgMask, hist2, 1, &histSize, ranges, true, false );
    cv::calcHist(&sourceChannels[2], 1, 0, fgMask, hist3, 1, &histSize, ranges, true, false );

    // Normalize
    normalize(hist1, hist1);
    normalize(hist2, hist2);
    normalize(hist3, hist3);
}

