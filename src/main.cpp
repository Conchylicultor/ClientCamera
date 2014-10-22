#include <iostream>
#include "opencv2/opencv.hpp"

#include "camera.h"

using namespace std;
using namespace cv;



int main()
{
    cout << "Loading camera..." << endl;

    /* ---- Instructions ----
     *
     * __Cameras__:
     * Login: root
     * Password: azerty
     *
     * __Urls__:
     * MotionJpg:  http://root:azerty@192.168.100.190/axis-cgi/mjpg/video.cgi?resolution=640x480;type=.mjpg
     * Mpeg4:      rtsp://root:azerty@192.168.100.191/mpeg4/media.amp
     * H.264:      rtsp://root:azerty@192.168.100.192/axis-media/media.amp?tcp
     * StillImage:
     *
     * __Wifi__:
     * PPID:wfpot WPA2:123456789
     * PPID:12345 WPA2:123456789
     *
     * __Recording__:
     * Set recording at true
     * Choose a destination directory in camera.cpp
     *
     *
     * */

    bool recording = false;

    Camera *cam1 = 0;
    Camera *cam2 = 0;
    Camera *cam3 = 0;

    cam1 = new Camera("/home/etienne/__A__/Data/Recordings/01_Vid_1.mp4",recording);
    cam2 = new Camera("/home/etienne/__A__/Data/Recordings/01_Vid_2.mp4",recording);
    cam3 = new Camera("/home/etienne/__A__/Data/Recordings/01_Vid_3.mp4",recording);
    //cam1 = new Camera("http://root:azerty@192.168.100.168/axis-cgi/mjpg/video.cgi?;type=.mjpg",recording);
    //cam2 = new Camera("http://root:azerty@192.168.100.169/axis-cgi/mjpg/video.cgi?;type=.mjpg",recording);
    //cam3 = new Camera("http://root:azerty@192.168.100.170/axis-cgi/mjpg/video.cgi?;type=.mjpg",recording);

    vector<Camera*> listCam;
    listCam.push_back(cam1);
    listCam.push_back(cam2);
    listCam.push_back(cam3);

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
