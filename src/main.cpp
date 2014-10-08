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

    /*int nthreads=0;             // number of thread available
    int threadid=0;             // id of thread

    #pragma omp parallel private(threadid)
    {
        nthreads = omp_get_num_threads(); // compute the total number of threads
        //cout << nthreads << " thread(s) available for computation"<<endl;
        threadid = omp_get_thread_num();
        //cout << "Thread " << threadid << " ready" <<endl;
    }*/

    bool recording = false;

    Camera *cam1 = 0;
    Camera *cam2 = 0;
    Camera *cam3 = 0;

    cam1 = new Camera("/home/etienne/__A__/Data/Recordings/02_Vid_1.mp4",recording);
    cam2 = new Camera("/home/etienne/__A__/Data/Recordings/02_Vid_2.mp4",recording);
    cam3 = new Camera("/home/etienne/__A__/Data/Recordings/02_Vid_3.mp4",recording);

    /*#pragma omp parallel sections
    {
        #pragma omp section
        {
            cam1 = new Camera("http://root:azerty@192.168.100.190/axis-cgi/mjpg/video.cgi?type=.mjpg",recording);
        }

        #pragma omp section
        {
            cam2 = new Camera("http://root:azerty@192.168.100.191/axis-cgi/mjpg/video.cgi?type=.mjpg",recording);
        }

        #pragma omp section
        {
            cam3 = new Camera("http://root:azerty@192.168.100.192/axis-cgi/mjpg/video.cgi?type=.mjpg",recording);
        }
    }*/

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
        //#pragma omp parallel for
        for(size_t i = 0 ; i < listCam.size() ; ++i)
        {
            listCam.at(i)->grab();
        }

        for(size_t i = 0 ; i < listCam.size() ; ++i)
        {
            listCam.at(i)->play();
        }

        if(waitKey(50) >= 0)
            break;
    }

    for(size_t i = 0 ; i < listCam.size() ; ++i)
    {
        delete listCam[i];
    }

    return 0;
}
