#include "camera.h"

// Arbitrary parametters
#define DETECT_MIN_HEIGHT 100
#define DETECT_MIN_AREA 1000
#define DETECT_MIN_FINAL_HEIGHT 60
#define DETECT_MIN_FINAL_WIDTH 20
#define DETECT_MIN_DIST_CLOSE 80

using namespace std;
using namespace cv;

int Camera::nbCams = 0;
ocl::HOGDescriptor *Camera::personDescriptor = nullptr;

std::list<MapDot> Camera::mapDotList;
cv::Mat Camera::mapBackground;
cv::VideoWriter Camera::mapTracesWriter;

Camera::Camera(string pathVid, int clientId, bool modeTrackingHog, bool record, bool hideGui) :
    hogMode(modeTrackingHog),
    recording(record),
    hidingGui(hideGui),
    success(false),
    pause(false),
    spacialLocalisation(false),
    backgroundSubstractor()
{
    if(hogMode && personDescriptor == nullptr) // First loading
    {
        personDescriptor = new ocl::HOGDescriptor();
        personDescriptor->setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());
    }

    cap.open(pathVid);

    if (!cap.isOpened())
    {
        cout << "Could not capture: " << endl;
        cout << pathVid << endl;
        exit(0);
    }

    nbCams++;
    nameVid = "Client_" + std::to_string(clientId) + "_Vid_" + std::to_string(nbCams);
    cout << nameVid << " loaded: " << pathVid << endl;


    if(!hidingGui)
    {
        namedWindow(nameVid);
        // moveWindow(nameVid, (nbCams-1)*640 ,0);
    }

    loadTransformationMatrix(); // To extract the 2d coordinates of the image

    // Sometimes the first frame is unreacheable
    while(!success)
    {
        grab();
        // TODO: Save the background image ?
    }

    if(recording)
    {
        if(success)
        {
            writer.open("../../Data/Recordings/" + nameVid + ".avi", CV_FOURCC('I','4','2','0'), 20, frame.size());
            if(!writer.isOpened())
            {
                cout << "Pb recording with " << nameVid << endl;
                recording = false;
            }
        }
        else
        {
            cout << "Pb recording with " << nameVid << endl;
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
    if (pause)
    {
        success=false;
        return;
    }
    else if (!cap.read(frame))
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

        // Pipeline
        detectPersons();
        tracking();
        computeFeatures();
        if(!hidingGui || recording) // Otherwise, no need to add computer time
        {
            addVisualInfos();
        }

        if(!hidingGui)
        {
            imshow(nameVid, frame);
        }

        if(recording)
        {
            writer << frame;
        }
    }
}

void Camera::togglePause()
{
    pause = !pause;
}

void Camera::loadTransformationMatrix()
{
    Mat bgImg = imread("../calibration/vid" + std::to_string(nbCams) + ".png");
    Mat mapImg = imread("../calibration/vid" + std::to_string(nbCams) + "_map.png");

    if(!bgImg.data || !mapImg.data)
    {
        cout << "Warning: cannot load the image for transformation matrix " << nbCams << endl;
        spacialLocalisation = false;
        return;
    }

    vector<Point2f> homographyPointsSrc(4);
    vector<Point2f> homographyPointsDest(4);

    int compteurDots = 0;

    Vec3b colorRed = Vec3b(0,0,255);
    Vec3b colorMagenta = Vec3b(255,0,255);
    Vec3b colorYellow = Vec3b(0,255,255);
    Vec3b colorGreen = Vec3b(0,255,0);

    for(int i = 0 ; i < bgImg.cols; ++i)
    {
        for(int j = 0 ; j < bgImg.rows; ++j)
        {
            Vec3b color = bgImg.at<Vec3b>(Point(i,j));

            if(color == colorRed)
            {
                homographyPointsSrc.at(0) = Point2f(i,j);
                compteurDots++;
            }
            else if(color == colorMagenta)
            {
                homographyPointsSrc.at(1) = Point2f(i,j);
                compteurDots++;
            }
            else if(color == colorYellow)
            {
                homographyPointsSrc.at(2) = Point2f(i,j);
                compteurDots++;
            }
            else if(color == colorGreen)
            {
                homographyPointsSrc.at(3) = Point2f(i,j);
                compteurDots++;
            }
        }
    }

    if(compteurDots != 4)
    {
        cout << "Error: Wrong number of dots for computing trasformation matrix (in background): " << compteurDots << endl;
        return;
    }

    compteurDots=0;

    for(int i = 0 ; i < mapImg.cols; ++i)
    {
        for(int j = 0 ; j < mapImg.rows; ++j)
        {
            Vec3b color = mapImg.at<Vec3b>(Point(i,j));

            if(color == colorRed)
            {
                homographyPointsDest.at(0) = Point2f(i,j);
                compteurDots++;
            }
            else if(color == colorMagenta)
            {
                homographyPointsDest.at(1) = Point2f(i,j);
                compteurDots++;
            }
            else if(color == colorYellow)
            {
                homographyPointsDest.at(2) = Point2f(i,j);
                compteurDots++;
            }
            else if(color == colorGreen)
            {
                homographyPointsDest.at(3) = Point2f(i,j);
                compteurDots++;
            }
        }
    }

    if(compteurDots != 4)
    {
        cout << "Error: Wrong number of dots for computing trasformation matrix (in map): " << compteurDots << endl;
        return;
    }

    homographyMatrix = findHomography(homographyPointsSrc, homographyPointsDest);

    spacialLocalisation = true;
}

void Camera::initMapTraces()
{
    mapBackground = imread("../calibration/map.png");
    if(!mapBackground.data)
    {
        cout << "Error: Cannot load map background"<< endl;
    }

    mapTracesWriter.open("../../Data/Traces/traces.avi", CV_FOURCC('I','4','2','0'), 20, mapBackground.size());
    if(!mapTracesWriter.isOpened())
    {
        cout << "Error: Pb recording with the map traces"<< endl;
    }
}

void Camera::updateMapTraces()
{
    // Uncomment to record the trace
    Mat currentFrame;// = mapBackground.clone(); // Uncomment

    mapDotList.remove_if([&currentFrame](MapDot &currentDot) -> bool {
        currentDot.update();
        if(currentDot.lifetime <= 0)
        {
            return true;
        }
        // circle(currentFrame, currentDot.position, 1, currentDot.color); // Uncomment
        return false;
    });

    // mapTracesWriter << currentFrame; // Uncomment
}

void Camera::closeMapTraces()
{
    mapTracesWriter.release();
}

void Camera::detectPersons()
{
    personsFound.clear();

    // Background detection
    backgroundSubstractor(frame, fgMask);

    if(hogMode)
    {
        // Extract the rois (foreground)

        // Detect countours
        Mat fgWithShadows = fgMask.clone();

        vector<vector<Point> > contoursWithShadows;
        cv::erode(fgWithShadows,fgWithShadows,cv::Mat());
        cv::dilate(fgWithShadows,fgWithShadows,cv::Mat());
        cv::findContours(fgWithShadows,contoursWithShadows, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

        // List which contains the regions of interest
        list<Rect> roiList;
        list<Rect> roiGroupList;
        static const int marginPerson = 5; // If two rects are close together
        static const int marginGroupPerson = 15; // If two persons are close together (avoid same person in two diferent roi)

        // Merge the persons
        for(const vector<Point> &countourPoints : contoursWithShadows)
        {
            Rect captureRect = cv::boundingRect(countourPoints);

            mergeCloseRoi(roiList, captureRect, marginPerson);
        }

        // Merge the groups
        for(Rect &rect : roiList)
        {
            // Filter noize (small size)
            if(rect.height > 40 && rect.width > 10)
            {
                mergeCloseRoi(roiGroupList, rect, marginGroupPerson);
            }
        }

        for(Rect &roi : roiGroupList)
        {
            roi = Rect(roi.tl() - Point(marginGroupPerson, marginGroupPerson),
                       roi.br() + Point(marginGroupPerson, marginGroupPerson)); // Add margins
            roi = roi & Rect(0, 0, frame.cols-1, frame.rows-1); // In case of outbreak

            extractPersons(frame(roi), roi.tl()); // Detect persons if possible
        }
    }
    else
    {
        // 3 Steps detections

        // First step: with shadow
        Mat fgWithShadows = fgMask.clone();

        std::vector<std::vector<cv::Point> > contoursWithShadows;
        cv::erode(fgWithShadows,fgWithShadows,cv::Mat());
        cv::dilate(fgWithShadows,fgWithShadows,cv::Mat());
        cv::findContours(fgWithShadows,contoursWithShadows, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

        // Second step: without shadow
        Mat fgWithoutShadows = fgMask; // No clone
        threshold(fgWithoutShadows, fgWithoutShadows, 254, 255, THRESH_BINARY);

        std::vector<std::vector<cv::Point> > contoursWithoutShadows;
        cv::erode(fgWithoutShadows,fgWithoutShadows,cv::Mat());
        cv::dilate(fgWithoutShadows,fgWithoutShadows,cv::Mat());
        cv::findContours(fgWithoutShadows,contoursWithoutShadows, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

        cv::drawContours(fgWithoutShadows,contoursWithoutShadows,-1,cv::Scalar(255), CV_FILLED);

        // Third step: extraction
        fgWithoutShadows = fgMask.clone();

        for(std::vector<std::vector<cv::Point>>::iterator iter = contoursWithShadows.begin() ; iter != contoursWithShadows.end(); iter++)
        {
            Rect captureRect = cv::boundingRect(*iter);
            // Filters (minimum height and area)
            // TODO: Aspect ratio to limit false positives ?
            // Add a perspective coefficient to make the minimum height can be proportional to the place on the camera
            if(captureRect.height > DETECT_MIN_HEIGHT && cv::contourArea(*iter) > DETECT_MIN_AREA)
            {
                // Working only on the small roi
                Mat persMask(fgWithoutShadows, captureRect);

                Point captureMinTl(persMask.size().width, persMask.size().height);
                Point captureMinBr(0, 0);// Inverted rect
                Rect captureCurrentRect;

                // Computing the big bounding box
                std::vector<std::vector<cv::Point> > contoursPers;
                cv::findContours(persMask, contoursPers, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
                for(std::vector<std::vector<cv::Point> >::iterator iterContoursPers = contoursPers.begin() ; iterContoursPers != contoursPers.end(); iterContoursPers++)
                {
                    captureCurrentRect = cv::boundingRect(*iterContoursPers);
                    captureMinTl.x = std::min(captureMinTl.x, captureCurrentRect.x);
                    captureMinTl.y = std::min(captureMinTl.y, captureCurrentRect.y);
                    captureMinBr.x = std::max(captureMinBr.x, captureCurrentRect.br().x);
                    captureMinBr.y = std::max(captureMinBr.y, captureCurrentRect.br().y);
                }

                // Ultimate filter: without shadow
                if(captureMinBr.x > DETECT_MIN_FINAL_WIDTH && captureMinBr.y > DETECT_MIN_FINAL_HEIGHT)
                {
                    captureRect.x += captureMinTl.x;
                    captureRect.y += captureMinTl.y;
                    captureRect.width  = captureMinBr.x - captureMinTl.x;
                    captureRect.height = captureMinBr.y - captureMinTl.y;
                    personsFound.push_back(captureRect);
                }
            }
        }
    }
}

void Camera::mergeCloseRoi(std::list<Rect> &roiList, Rect &newRect, int margin) const
{
    // Add margins
    Rect rectWithMargin(newRect.tl() - Point(margin, margin),
                        newRect.br() + Point(margin, margin));

    // Delete all rect which are merged
    roiList.remove_if(
        [&newRect, &rectWithMargin](Rect &previousRect)
        {
            if((rectWithMargin & previousRect) != Rect()) // If close enough (intersection not null)
            {
                // We merge the two rects
                newRect = newRect | previousRect;

                return true; // Remove the rect
            }
            return false;
        }
    );

    // Add the new merged rect
    roiList.push_back(newRect);
}

void Camera::extractPersons(const Mat &roi, const Point &roiPos)
{
    static const int scaleFactor = 2;
    vector<Rect> hogFound;
    vector<Rect> hogFoundFiltered;

    // Detection
    Mat greyFrame;
    ocl::oclMat oclFrame;

    cv::cvtColor(roi, greyFrame, CV_BGR2GRAY); // Keep only one channel
    ocl::resize(ocl::oclMat(greyFrame), oclFrame, greyFrame.size()*scaleFactor); // We scale X2 to detect smaller people

    if(oclFrame.rows > 128 && oclFrame.cols > 64) // Minimum detection size
    {
        personDescriptor->detectMultiScale(oclFrame, hogFound);
    }

    // Remove the duplicate (code from internet so usefull ?)
    size_t j;
    for (size_t i = 0 ; i < hogFound.size() ; i++)
    {
        Rect r = hogFound[i];
        for (j = 0; j<hogFound.size(); j++)
        {
            if (j != i && (r & hogFound[j])==r)
            {
                break;
            }
        }
        if (j==hogFound.size())
        {
            hogFoundFiltered.push_back(r);
        }
    }

    // Extraction
    for (Rect &r : hogFoundFiltered)
    {
        r.x /= scaleFactor;
        r.y /= scaleFactor;
        r.width /= scaleFactor;
        r.height /= scaleFactor; // Resize

        r.x += roiPos.x;
        r.y += roiPos.y; // Offset of the roi

        personsFound.push_back(r);
    }
}

void Camera::tracking()
{
    // Reset update statuts
    for(list<Silhouette*>::iterator iter = listCurrentSilhouette.begin() ; iter != listCurrentSilhouette.end() ; iter++)
    {
        (*iter)->setUpdated(false);// For the next loop
    }

    // For each person found
    for (size_t i=0; i < personsFound.size(); i++)
    {
        // Algorithm:
        // We looking the closest existing silhouette which exist in the list
        // If found, we associate this silhouette to the current detected person
        // If not, we add it to the list

        // Looking for the closest silhouette
        int minDist = 0;
        Silhouette *closestSilhouette = 0;
        for(list<Silhouette*>::iterator iter = listCurrentSilhouette.begin() ; iter != listCurrentSilhouette.end() ; iter++)
        {
            if(!(*iter)->getUpdated())// The silhouette has not been computed yet
            {
                if(closestSilhouette == 0) // First element
                {
                    closestSilhouette = *iter;
                    minDist = closestSilhouette->distanceFrom(personsFound[i]);
                }
                else
                {
                    // Is there a closest element ?
                    if((*iter)->distanceFrom(personsFound[i]) < minDist)
                    {
                        closestSilhouette = *iter;// We update the closest silhouette
                        minDist = (*iter)->distanceFrom(personsFound[i]);
                    }
                }
            }
        }

        // Validating
        if(closestSilhouette == 0) // New element
        {
            Silhouette *silh = new Silhouette;
            silh->addPos(personsFound[i]);
            silh->setUpdated(true);
            listCurrentSilhouette.push_back(silh);
        }
        else if(minDist < DETECT_MIN_DIST_CLOSE) // Person close => same person
        {
            closestSilhouette->addPos(personsFound[i]);
            closestSilhouette->setUpdated(true);
        }
        else // Person far => New element
        {
            Silhouette *silh = new Silhouette;
            silh->addPos(personsFound[i]);
            silh->setUpdated(true);
            listCurrentSilhouette.push_back(silh);
        }
    }

    // Update the silhouettes
    for(list<Silhouette*>::iterator iter = listCurrentSilhouette.begin() ; iter != listCurrentSilhouette.end() ; ) // /!\ Warning: No incrementation at the end (elements can be deleted)
    {
        if(!(*iter)->getUpdated())// The silhouette has not been computed yet
        {
            // 1st case, the silhouette was a false positiv
            // 2nd case, the silhouette has disappear
            if((*iter)->getGostLife() == -1)
            {
                (*iter)->setGostLife(7);
            }
            else if((*iter)->getGostLife() > 0) // The silhouette is lost (but not removed yet)
            {
                (*iter)->setGostLife((*iter)->getGostLife() - 1);
            }
            else if((*iter)->getGostLife() == 0) // Time out. The silhouette is removed (/!\ validity of the iterator)
            {
                (*iter)->saveCamInfos(nameVid, homographyMatrix); // We save the camera information before destroying the object
                delete (*iter);
                iter = listCurrentSilhouette.erase(iter);
                continue;
            }
        }
        else
        {
            // New apparence, it is not a false positiv
            if((*iter)->getGostLife() != -1) // Else it is not a gost (Retrouve, ouf!)
            {
                (*iter)->setGostLife(-1);
            }
        }
        iter++;
    }
}

void Camera::computeFeatures()
{
    // For each silhouette
    for(list<Silhouette*>::iterator iter = listCurrentSilhouette.begin() ; iter != listCurrentSilhouette.end() ; iter++)
    {
        if((*iter)->getUpdated())// The silhouette is present on the current frame
        {
            // TODO: Other conditions to extract the features ?
            (*iter)->addFrame(frame, fgMask, spacialLocalisation, homographyMatrix, mapDotList);
        }
    }
}

void Camera::addVisualInfos()
{
    // Plot a frame above the detected persons (detection level)
    /*for (size_t i=0; i < personsFound.size(); i++)
    {
        rectangle(frame, personsFound[i], cv::Scalar(0,255,0), 2);

        //Mat persMask(fgMask, personsFound[i]);
        //imshow("Detected: " + nameVid, persMask);
    }*/

    // Plot a frame above the detected persons (tracking level)
    for(list<Silhouette*>::iterator iter = listCurrentSilhouette.begin() ; iter != listCurrentSilhouette.end() ; iter++)
    {
        (*iter)->plot(frame);
    }
}
