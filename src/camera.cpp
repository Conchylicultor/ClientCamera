#include "camera.h"

// Arbitrary parametters
#define DETECT_MIN_HEIGHT 100
#define DETECT_MIN_AREA 1000

int Camera::nbCams = 0;

Camera::Camera(string pathVid, bool record) :
    recording(record),
    success(false),
    backgroundSubstractor()
{
    cap.open(pathVid);

    if (!cap.isOpened())
    {
        cout << "Could not capture :" << endl;
        cout << pathVid << endl;
        exit(0);
    }

    nbCams++;
    nameVid = "Vid_" + std::to_string(nbCams);
    cout << nameVid << " loaded" << endl;

    namedWindow(nameVid);

    // Sometimes the first frame is unreacheable
    while(!success)
    {
        grab();
    }

    if(recording)
    {
        if(success)
        {
            writer.open("/home/etienne/__A__/Data/Recordings/" + nameVid + ".avi", CV_FOURCC('I','4','2','0'), 20, frame.size());
            if(!writer.isOpened())
            {
                cout << "Pb recording with" << nameVid << endl;
                recording = false;
            }
        }
        else
        {
            cout << "Pb recording with" << nameVid << endl;
        }
    }

    // Pipeline
}

Camera::~Camera()
{
    cap.release();
    writer.release();
}

void Camera::grab()
{
    if (!cap.read(frame))
    {
        std::cout << "Unable to retrieve frame from " << nameVid << std::endl;
        success=false;
        return;
    }
    success=true;
}

void Camera::play()
{
    if(success)
    {
        //resize(frame, frame, Size(RED_WIDTH,RED_HEIGHT));

        detectPersons();
        addVisualInfos();

        imshow(nameVid, frame);

        if(recording)
        {
            writer << frame;
        }
    }
}

void Camera::detectPersons()
{
    // Background detection
    backgroundSubstractor(frame, fgMask);

    personsFound.clear();

    // 2 Steps detections
    // First step: with shadow
    Mat fgWithShadows = fgMask.clone();

    std::vector<std::vector<cv::Point> > contoursWithShadows;
    cv::erode(fgWithShadows,fgWithShadows,cv::Mat());
    cv::dilate(fgWithShadows,fgWithShadows,cv::Mat());
    cv::findContours(fgWithShadows,contoursWithShadows, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

    // Second step: without shadow
    Mat fgWithoutShadows = fgMask.clone();
    threshold(fgWithoutShadows, fgWithoutShadows, 254, 255, THRESH_BINARY);

    std::vector<std::vector<cv::Point> > contoursWithoutShadows;
    cv::erode(fgWithoutShadows,fgWithoutShadows,cv::Mat());
    cv::dilate(fgWithoutShadows,fgWithoutShadows,cv::Mat());
    cv::findContours(fgWithoutShadows,contoursWithoutShadows, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

    cv::drawContours(fgWithoutShadows,contoursWithoutShadows,-1,cv::Scalar(255), CV_FILLED);

    // Third step: extraction
    for(std::vector<std::vector<cv::Point> >::iterator iter = contoursWithShadows.begin() ; iter != contoursWithShadows.end(); iter++)
    {
        Rect captureRect = cv::boundingRect(*iter);
        // Filters (minimum height and area)
        if(captureRect.height > DETECT_MIN_HEIGHT && cv::contourArea(*iter) > DETECT_MIN_AREA)
        {
            personsFound.push_back(captureRect);

            Mat persMask(fgWithoutShadows, captureRect);
            imshow("rrrt"+nameVid, persMask);

        }
    }

    // Second step: without shadow
    /*for(std::vector<std::vector<cv::Point> >::iterator iter = contoursWithShadows.begin() ; iter != contoursWithShadows.end(); iter++)
    {
        Rect captureRect = cv::boundingRect(*iter);
        // Filters (minimum height and area)
        if(captureRect.height > DETECT_MIN_HEIGHT && cv::contourArea(*iter) > DETECT_MIN_AREA)
        {
            // Detects contours without shadow
            Mat fgMaskWithoutShadows(fgMask, captureRect);
            threshold(fgMaskWithoutShadows, fgMaskWithoutShadows, 254, 255, THRESH_BINARY);
            imshow("rrrt"+nameVid, fgMaskWithoutShadows);

            std::vector<std::vector<cv::Point> > contoursWithoutShadows;
            cv::erode(fgMaskWithoutShadows,fgMaskWithoutShadows,cv::Mat());
            cv::dilate(fgMaskWithoutShadows,fgMaskWithoutShadows,cv::Mat());
            cv::findContours(fgMaskWithoutShadows,contoursWithoutShadows, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

            // We compute the total area of the person
            // And the small rect area ???
            int areaPers = 0;

            int x1 = captureRect.width;
            int y1 = captureRect.height; // We are looking for the minimum value (so we give the max value)
            int x2 = 0;
            int y2 = 0;
            for(std::vector<std::vector<cv::Point> >::iterator iterContour = contoursWithoutShadows.begin() ; iterContour != contoursWithoutShadows.end(); iterContour++)
            {
                areaPers += (int)cv::contourArea(*iterContour);
                for(std::vector<cv::Point>::iterator iterPoints = iterContour->begin() ; iterPoints != iterContour->end() ; iterPoints++)
                {
                    if(iterPoints->x < x1)
                    {
                        x1 = iterPoints->x;
                    }
                    if(iterPoints->y < y1)
                    {
                        y1 = iterPoints->y;
                    }
                    if(iterPoints->x > x2)
                    {
                        x2 = iterPoints->x;
                    }
                    if(iterPoints->y > y2)
                    {
                        y2 = iterPoints->y;
                    }
                }
            }
            personsFound.push_back(Rect(x1, y1, x2-x1, y2-y1));
            personsFound.back().x += captureRect.x;
            personsFound.back().y += captureRect.y;
        }
    }
    cv::drawContours(frame, contoursWithShadows,-1,cv::Scalar(255));*/
}

void Camera::addVisualInfos()
{
    // Plot a frame above the detected persons
    for (size_t i=0; i < personsFound.size(); i++)
    {
        Rect r = personsFound[i];
        rectangle(frame, r.tl(), r.br(), cv::Scalar(0,255,0), 2);
    }
}
