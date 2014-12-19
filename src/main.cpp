#include <iostream>
#include "opencv2/opencv.hpp"

#include "camera.h"
#include "silhouette.h"

using namespace std;
using namespace cv;



int main()
{
    cout << "Loading camera..." << endl;

    bool recording = false;
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

    if(!fileConfig["recordingTrace"].empty())
    {
        bool recordingTrace = false;
        fileConfig["recordingTrace"] >> recording;
        Silhouette::setRecordTrace(recordingTrace);
    }

    FileNode nodeVideoNames = fileConfig["videoNames"];

    cout << "Try opening " << nodeVideoNames.size() << " sources..." << endl;
    for(string currentName : nodeVideoNames)
    {
        listCam.push_back(new Camera(currentName, recording));
    }

    // Clear the buffer of each cam
    /*for(int i = 0 ; i < 150 ; ++i)
    {
        for(int j = 0 ; j < listCam.size() ; ++j)
        {
            listCam.at(j)->grab();
        }
    }*/

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
    }

    for(size_t i = 0 ; i < listCam.size() ; ++i)
    {
        delete listCam[i];
    }

    return 0;
}
