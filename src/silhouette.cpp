#include "silhouette.h"

#include <iostream>
#include <fstream>

using namespace std;
using namespace cv;

// We only save if the person has been recorded more than 5 times (frames)
const int nbFilterFirstFrame = 3; // The first frames are usually not very good
const int minToSave = 6;

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
    // Smooth tracking (hack if not in hog mode)
    /*if(previousPos.size() > 0)
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
    }*/

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

void Silhouette::addFrame(Mat &frame, Mat &fgMask, bool useHomographyMatrix, const Mat &homographyMatrix, list<MapDot> &mapDotList)
{
    Mat persImg  = frame (previousPos.back());
    Mat persMask = fgMask(previousPos.back());

    extFrames.push_back(pair<Mat, Mat>(persImg, persMask)); // We always add the image to the list (but we don't save it if it is filtered)

    // Save on disk (for built the database)
    // We only save if the person has been recorded more than x times (frames)
    if(recordTrace && extFrames.size() >= minToSave)
    {
        // Compute image id
        string sequenceId  = std::to_string(clientId) + "_" + std::to_string(id); // Id of the complete sequence

        // The sequence image id are recorded on a separate file
        ofstream fileSequenceTrace("../../Data/Traces/" + sequenceId + "_list.txt", ios_base::app);
        if(!fileSequenceTrace.is_open())
        {
            cout << "Error: Cannot record the traces (Right folder ?)" << endl;
        }

        // Function to save one image (with its mask and update the sequence image id list)
        auto fuctionRecordId = [&fileSequenceTrace] (const string &imageIdToSave, const pair<Mat, Mat> &imagesToSave) -> void {

            // Filter: aspect ratio (for non hog mod)
            // TODO: Add also a filter if not enough foreground pixel
            // TODO: Add also a filter if foreground not in center of picture ? (person probably cropped)
            bool filter = true;
            if(imagesToSave.first.rows / imagesToSave.first.cols > 1.4)
            {
                filter = false;
            }

            if(!filter)
            {
                // Save sequence images id
                fileSequenceTrace << imageIdToSave << endl;

                // Save image
                imwrite("../../Data/Traces/" + imageIdToSave + ".png", imagesToSave.first);
                imwrite("../../Data/Traces/" + imageIdToSave + "_mask.png", imagesToSave.second);
            }

        };

        // First time, we save all previous frame
        if(extFrames.size() == minToSave)
        {
            // Save the sequence id

            ofstream fileTracesOut;
            fileTracesOut.open ("../../Data/Traces/traces.txt", ios_base::app);
            if(!fileTracesOut.is_open())
            {
                cout << "Error: No file trace out" << endl;
            }
            fileTracesOut << "----- " + sequenceId + " -----" << endl;
            fileTracesOut << sequenceId + "_cam" << endl;
            if(useHomographyMatrix)
            {
                fileTracesOut << sequenceId + "_pos" << endl;
            }
            fileTracesOut.close();

            // Save all previous frame

            for(size_t i = nbFilterFirstFrame ; i < extFrames.size() ; ++i)
            {
                string imageId = sequenceId + "_" + std::to_string(i); // Index start at 0
                fuctionRecordId(imageId, extFrames.at(i));
            }
        }
        // The next frames
        else if(extFrames.size() > minToSave)
        {
            string imageId = sequenceId + "_" + std::to_string(extFrames.size()-1); // Id of the image inside the sequence
            fuctionRecordId(imageId, extFrames.back());
        }

        fileSequenceTrace.close();

        // Compute homography

        if(homographyMatrix.data)
        {
            // Compute coordinate
            Mat camCoordinate = Mat::ones(3,1, CV_64F);
            const Rect &currentPos = previousPos.back();
            camCoordinate.at<double>(0) = currentPos.br().x - currentPos.width/2;
            camCoordinate.at<double>(1) = currentPos.br().y;

            Mat mapCoordinate = homographyMatrix * camCoordinate;
            mapCoordinate /= mapCoordinate.at<double>(2); // Convert to cartesian coordinates

            // Plot the path on the map
            mapDotList.push_back(MapDot(Point(mapCoordinate.at<double>(0), mapCoordinate.at<double>(1)), color));
        }
    }
}

void Silhouette::saveCamInfos(string nameVid, const cv::Mat &homographyMatrix)
{
    if(recordTrace && extFrames.size() >= minToSave) // Filter the false positive
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
            Mat tracesMap = imread("../../Data/Traces/map.png");
            if(!tracesMap.data)
            {
                tracesMap = imread("../calibration/map.png");
                if(!tracesMap.data)
                {
                    cout << "Warning: no camera map, cannot record the trace map" << endl;
                    return;
                }
            }

            ofstream fileTracePos("../../Data/Traces/" + std::to_string(clientId) + "_" + std::to_string(id) + "_pos.txt");
            for(const Rect &currentPos : previousPos)
            {
                // Compute coordinate
                Mat camCoordinate = Mat::ones(3,1, CV_64F);
                camCoordinate.at<double>(0) = currentPos.br().x - currentPos.width/2;
                camCoordinate.at<double>(1) = currentPos.br().y;

                Mat mapCoordinate = homographyMatrix * camCoordinate;
                mapCoordinate /= mapCoordinate.at<double>(2); // Convert to cartesian coordinates

                // Plot the path on the map
                circle(tracesMap, Point(mapCoordinate.at<double>(0), mapCoordinate.at<double>(1)), 1, color);

                // Save on file
                fileTracePos << mapCoordinate.at<double>(0) << " " << mapCoordinate.at<double>(1) << endl;
            }
            fileTracePos.close();

            imwrite("../../Data/Traces/map.png", tracesMap);
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
