#include <iostream>
#include <chrono>
#include "opencv2/opencv.hpp"

#include "camera.h"
#include "silhouette.h"

using namespace std;
using namespace cv;



int main()
{
    cout << "Loading camera..." << endl;

    int recording = 0;
    int hideGui = 0;
    chrono::minutes lifeTime(-1);
    vector<Camera*> listCam;

    // Set up configuration
    FileStorage fileConfig("../video.yml", FileStorage::READ);
    if(!fileConfig.isOpened())
    {
        cout << "Error: No congiguration file" << endl;
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
        Silhouette::setRecordTrace(recordingTrace);
    }

    FileNode nodeVideoNames = fileConfig["videoNames"];

    cout << "Try opening " << nodeVideoNames.size() << " sources..." << endl;
    for(string currentName : nodeVideoNames)
    {
        listCam.push_back(new Camera(currentName, recording, hideGui));
    }

    // Clear the buffer of each cam
    /*for(int i = 0 ; i < 150 ; ++i)
    {
        for(int j = 0 ; j < listCam.size() ; ++j)
        {
            listCam.at(j)->grab();
        }
    }*/

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

    for(size_t i = 0 ; i < listCam.size() ; ++i)
    {
        delete listCam[i];
    }

    return 0;
}
