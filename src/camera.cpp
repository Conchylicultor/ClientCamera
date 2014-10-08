#include "camera.h"

int Camera::nbCams = 0;

Camera::Camera(string pathVid, bool record) :
    recording(record),
    success(false)
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
