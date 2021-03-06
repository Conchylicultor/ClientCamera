#include <iostream>
#include <chrono>
#include "opencv2/opencv.hpp"
#include "opencv2/ocl/ocl.hpp"

#include "camera.h"
#include "silhouette.h"

using namespace std;
using namespace cv;



int main()
{
    cout << "Loading camera..." << endl;

    int hogMode = 0;
    int recording = 0;
    int hideGui = 0;
    int clientId = 0;
    chrono::minutes lifeTime(-1);
    vector<Camera*> listCam;

    // Set up configuration
    FileStorage fileConfig("../../ClientOnlineCamera/config.yml", cv::FileStorage::READ); // For reading the client id
    if(fileConfig.isOpened())
    {
        if(!fileConfig["clientId"].empty())
        {
            fileConfig["clientId"] >> clientId;
        }
    }

    if(!clientId)
    {
        cout << "Warning, no client id found, use the default one (0)" << endl;
    }
    Silhouette::setClientId(clientId);
    fileConfig.release();

    fileConfig.open("../video.yml", FileStorage::READ);
    if(!fileConfig.isOpened())
    {
        cout << "Error: No congiguration file" << endl;
    }

    if(!fileConfig["trackingHog"].empty())
    {
        fileConfig["trackingHog"] >> hogMode;
    }

    if(hogMode)
    {
        cout << "Mode hog active" << endl;
        cv::ocl::DevicesInfo devices;
        cv::ocl::getOpenCLDevices(devices);
        cv::ocl::setDevice(devices[0]);
    }

    if(!fileConfig["recordingVid"].empty())
    {
        fileConfig["recordingVid"] >> recording;
    }

    if(!fileConfig["hideGui"].empty())
    {
        fileConfig["hideGui"] >> hideGui;
    }

    if(!fileConfig["lifeTime"].empty())
    {
        int lifeTimeInt = 0;
        fileConfig["lifeTime"] >> lifeTimeInt;
        lifeTime = chrono::minutes(lifeTimeInt);
    }

    if(!fileConfig["recordingTrace"].empty())
    {
        int recordingTrace = 0;
        fileConfig["recordingTrace"] >> recordingTrace;
        if(recordingTrace)
        {
            system("exec rm ../../Data/Traces/*"); // Clear the folder before recording
        }
        Silhouette::setRecordTrace(recordingTrace);
    }

    FileNode nodeVideoNames = fileConfig["videoNames"];

    cout << "Try opening " << nodeVideoNames.size() << " sources..." << endl;
    for(string currentName : nodeVideoNames)
    {
        listCam.push_back(new Camera(currentName, clientId, hogMode, recording, hideGui));
    }

    fileConfig.release();

    Camera::initMapTraces();

    chrono::steady_clock::time_point beginTime = chrono::steady_clock::now();
    while(1)
    {
        // Computation
        for(size_t i = 0 ; i < listCam.size() ; ++i)
        {
            listCam.at(i)->grab();
        }

        for(size_t i = 0 ; i < listCam.size() ; ++i)
        {
            listCam.at(i)->play();
        }

        Camera::updateMapTraces();

        // Events
        char key = waitKey(50);
        if(key == 32) // Spacebar
        {
            for(size_t i = 0 ; i < listCam.size() ; ++i)
            {
                listCam.at(i)->togglePause();
            }
        }
        else if(key >= 0) // Other key
        {
            break; // Exit
        }

        if(lifeTime > chrono::minutes(0) &&
           chrono::steady_clock::now()-beginTime > lifeTime)
        {
            break; // Exit
        }
    }

    Camera::closeMapTraces();

    for(size_t i = 0 ; i < listCam.size() ; ++i)
    {
        delete listCam[i];
    }

    return 0;
}
