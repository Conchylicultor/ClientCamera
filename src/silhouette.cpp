#include "silhouette.h"

#include <iostream>
#include <fstream>


// We only save if the person has been recorded more than 5 times (frames)
const int minToSave = 5;

// Entrance and exit direction vectors are compute using this number of frame (has to be < minToSave)
const int nbFrameDirection = 4;

bool Silhouette::recordTrace = false;
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

    if(recordTrace)
    {
        beginTime = chrono::system_clock::now();
    }
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

void Silhouette::addFrame(Mat &frame, Mat &fgMask)
{
    // (?) Other conditions to extract the features ?
    Mat persImg  = frame (previousPos.back());
    Mat persMask = fgMask(previousPos.back());

    // Save on disk (for built the database)
    if(recordTrace)
    {
        // Save on disk >> For the database
        extFrames.push_back(pair<Mat, Mat>(persImg, persMask));

        // We only save if the person has been recorded more than 10 times (frames)
        if(extFrames.size() > minToSave)
        {
            // Filter: aspect ratio
            if(extFrames.back().first.rows / extFrames.back().first.cols > 1.4)
            {
                // Compute image id
                string imageId = std::to_string(id) + "_" + std::to_string(extFrames.size());

                // Save image
                imwrite("../../Data/Traces/" + imageId + ".png", extFrames.back().first);
                imwrite("../../Data/Traces/" + imageId + "_mask.png", extFrames.back().second);

                list<string> contentTraces;

                // Look into the file
                ifstream fileTracesIn;
                fileTracesIn.open ("../../Data/Traces/traces.txt");
                if(fileTracesIn.is_open())
                {
                    // Read entire file
                    for(string line ; std::getline(fileTracesIn, line) ; )
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
                    contentTraces.push_back(std::to_string(id) + "_cam");
                    // The camera id information is added in an annex file when the sequence is finished
                }
                else
                {
                    iter++;
                    contentTraces.insert(iter, imageId);
                }

                // Write file
                ofstream fileTracesOut;
                fileTracesOut.open ("../../Data/Traces/traces.txt");
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

void Silhouette::saveCamInfos(string nameVid)
{
    if(recordTrace && extFrames.size() > (minToSave+1))
    {
        // Compute the needed informations

        chrono::system_clock::time_point endTime = chrono::system_clock::now();

        if(extFrames.size() < nbFrameDirection)
        {
            cout << "Error: no enought frame to compute the direction" << endl;
            exit(0);
        }

        // Entrance vector
        cv::Point pt11(previousPos.front().x + previousPos.front().width/2,
                      previousPos.front().y + previousPos.front().height/2);
        cv::Point pt12(previousPos.at(nbFrameDirection).x + previousPos.at(nbFrameDirection).width/2,
                      previousPos.at(nbFrameDirection).y + previousPos.at(nbFrameDirection).height/2);

        // Exit vector
        cv::Point pt21(previousPos.at(previousPos.size() - nbFrameDirection - 1).x + previousPos.at(previousPos.size() - nbFrameDirection - 1).width/2,
                      previousPos.at(previousPos.size() - nbFrameDirection - 1).y + previousPos.at(previousPos.size() - nbFrameDirection - 1).height/2);
        cv::Point pt22(previousPos.back().x + previousPos.back().width/2,
                      previousPos.back().y + previousPos.back().height/2);

        // Recording

        FileStorage fileTraceCam("../../Data/Traces/" + std::to_string(id) + "_cam.yml", FileStorage::WRITE);
        if(!fileTraceCam.isOpened())
        {
            cout << "Error: Failed to save the camera informations" << endl;
            return;
        }
        fileTraceCam << "camId" << nameVid;
        fileTraceCam << "entranceVector" << "{";
            fileTraceCam << "x1" << pt11.x;
            fileTraceCam << "y1" << pt11.y;
            fileTraceCam << "x2" << pt12.x;
            fileTraceCam << "y2" << pt12.y;
            // The normalized vector (only the direction with enventually a gama information) will be compute in the other clients
        fileTraceCam << "}";
        fileTraceCam << "exitVector" << "{";
            fileTraceCam << "x1" << pt21.x;
            fileTraceCam << "y1" << pt21.y;
            fileTraceCam << "x2" << pt22.x;
            fileTraceCam << "y2" << pt22.y;
        fileTraceCam << "}";
        fileTraceCam << "beginDate" << static_cast<int>(chrono::system_clock::to_time_t(beginTime));
        fileTraceCam << "endDate" << static_cast<int>(chrono::system_clock::to_time_t(endTime));
        fileTraceCam.release();

        // Debug code:
        /*cv::Mat testImg(1000,1000, CV_8UC3, Scalar(0,18,19));
        cv::line(testImg, pt11, pt12, color, 2);
        cv::circle(testImg, pt12, 3, color);
        cv::line(testImg, pt21, pt22, color, 2);
        cv::circle(testImg, pt22, 3, color);
        cv::imshow("Direction vector", testImg);*/
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

void Silhouette::setRecordTrace(bool value)
{
    recordTrace = value;
}
