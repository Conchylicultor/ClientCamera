#include "silhouette.h"

#include <iostream>
#include <fstream>

using namespace std;
using namespace cv;

// We only save if the person has been recorded more than 5 times (frames)
const int minToSave = 5;

// Limit ratio for smooth tracking
const float maxRatioHeight = 1.1;
const float correctionRatioHeightStepDown = 0.02;

// Entrance and exit direction vectors are compute using this number of frame (has to be < minToSave)
const int nbFrameDirectionMin = 4;
const float lengthDirectionVectorThreshold = 50.0;

bool Silhouette::recordTrace = false;
int Silhouette::clientId = 0;
int Silhouette::nbIds = 0;

cv::Point centerRect(const cv::Rect &rect)
{
    return cv::Point(rect.x + rect.width/2,
                     rect.y + rect.height/2);
}

float euclideanDist(const cv::Point &p, const cv::Point &q) {
    cv::Point diff = p - q;
    return cv::sqrt(diff.x*diff.x + diff.y*diff.y);
}

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
    cv::Point c1(centerRect(rect));
    cv::Point c2(centerRect(previousPos.back()));

    return (int)euclideanDist(c1, c2);
}

void Silhouette::addPos(Rect &newPos)
{
    // Smooth tracking
    if(previousPos.size() > 0)
    {
        float ratioHeight = static_cast<float>(previousPos.back().height) / static_cast<float>(newPos.height);
        if(ratioHeight > maxRatioHeight)
        {
            // Recompute the rect
            float correctionRatioHeight = 1.0;
            correctionRatioHeight -= correctionRatioHeightStepDown;

            int correctHeight = previousPos.back().height * correctionRatioHeight;

            if(std::abs(newPos.y - previousPos.back().y) >= std::abs((newPos.y + newPos.height) - (previousPos.back().y + previousPos.back().height))) // Head cropped
            {
                newPos.y = newPos.y + newPos.height - correctHeight;
            } // Otherwise, foot cropped
            newPos.height = correctHeight;

            // Check if the rect is really inside the image
            if(newPos.y <= 0)
            {
                newPos.y = 0;
            }
            if(newPos.y + newPos.height >= 380) // TODO: Extract the real screen size
            {
                newPos.height = 380 - newPos.y;
            }
        }
    }

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

void Silhouette::addFrame(Mat &frame, Mat &fgMask, bool useHomographyMatrix)
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
                string sequenceId  = std::to_string(clientId) + "_" + std::to_string(id); // Id of the complete sequence
                string imageId = sequenceId + "_" + std::to_string(extFrames.size()); // Id of the image inside the sequence

                // Save image
                imwrite("../../Data/Traces/" + imageId + ".png", extFrames.back().first);
                imwrite("../../Data/Traces/" + imageId + "_mask.png", extFrames.back().second);

                list<string> contentTraces;

                // We save all the sequences
                ofstream fileSequenceTrace("../../Data/Traces/" + sequenceId + "_list.txt", ios_base::app);
                if(!fileSequenceTrace.is_open())
                {
                    cout << "Error: Cannot record the traces (Right folder ?)" << endl;
                }
                fileSequenceTrace << imageId << endl;
                fileSequenceTrace.close();

                // TODO: Only record the main sequence id:

                // Look into the file
                ifstream fileTracesIn;
                fileTracesIn.open ("../../Data/Traces/traces.txt");
                if(fileTracesIn.is_open()) // For the first sequence, the file may not exist
                {
                    // Read entire file
                    for(string line ; std::getline(fileTracesIn, line) ; )
                    {
                        contentTraces.push_back(line);
                    }
                    fileTracesIn.close();
                }

                // Add content: insert lines
                string titleId = "----- " + sequenceId + " -----";
                list<string>::iterator iter = std::find(contentTraces.begin(), contentTraces.end(), titleId);

                if(iter == contentTraces.end())
                {
                    contentTraces.push_back(titleId);
                    contentTraces.push_back(imageId);
                    contentTraces.push_back(sequenceId + "_cam");
                    if(useHomographyMatrix)
                    {
                        contentTraces.push_back(sequenceId + "_pos");
                    }
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

void Silhouette::saveCamInfos(string nameVid, const cv::Mat &homographyMatrix)
{
    if(recordTrace && extFrames.size() > (minToSave+1)) // Filter the false positive
    {
        // Compute the needed informations

        chrono::system_clock::time_point endTime = chrono::system_clock::now();

        if(extFrames.size() < nbFrameDirectionMin)
        {
            cout << "Error: no enought frame to compute the direction" << endl;
            exit(0);
        }

        float vectorLength = 0.0;

        // Entrance vector
        cv::Point pt11(centerRect(previousPos.front()));
        cv::Point pt12;
        for(size_t i = nbFrameDirectionMin ; i < previousPos.size() ; ++i) // Try to compute a significant vector (length > threshold)
        {
            pt12 = Point(centerRect(previousPos.at(i)));
            vectorLength = euclideanDist(pt11, pt12);

            // End of the loop if the vector is long enought
            if(vectorLength > lengthDirectionVectorThreshold)
            {
                i = previousPos.size();
            }
        }

        if(vectorLength < lengthDirectionVectorThreshold)
        {
            cout << "Warning: direction vector length too small" << endl;
        }

        // Exit vector
        cv::Point pt21;
        cv::Point pt22(centerRect(previousPos.back()));
        for(size_t i = nbFrameDirectionMin ; i < previousPos.size() ; ++i) // Try to compute a significant vector (length > threshold)
        {
            pt21 = Point(centerRect(previousPos.at(previousPos.size() - i - 1)));
            vectorLength = euclideanDist(pt21, pt22);

            // End of the loop if the vector is long enought
            if(vectorLength > lengthDirectionVectorThreshold)
            {
                i = previousPos.size();
            }
        }

        if(vectorLength < lengthDirectionVectorThreshold)
        {
            cout << "Warning: direction vector length too small" << endl;
        }

        // Recording

        FileStorage fileTraceCam("../../Data/Traces/" + std::to_string(clientId) + "_" + std::to_string(id) + "_cam.yml", FileStorage::WRITE);
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

        // Homography computation
        if(homographyMatrix.data)
        {
            FileStorage fileTracePos("../../Data/Traces/" + std::to_string(clientId) + "_" + std::to_string(id) + "_pos.yml", FileStorage::WRITE);
            for(const Rect &currentPos : previousPos)
            {
                Mat camCoordinate = Mat::ones(3,1, CV_64F);
                camCoordinate.at<double>(0) = currentPos.br().x - currentPos.width/2;
                camCoordinate.at<double>(1) = currentPos.br().y;

                Mat mapCoordinate = homographyMatrix * camCoordinate;
                mapCoordinate /= mapCoordinate.at<double>(2); // Convert to cartesian coordinates

                // TODO: Save
            }
            fileTracePos.release();
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

void Silhouette::setRecordTrace(bool value)
{
    recordTrace = value;
}

void Silhouette::setClientId(int value)
{
    clientId = value;
}
