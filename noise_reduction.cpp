#include "noise_reduction.hpp"
#include <iostream>
#include <fstream>

void load_resolutions(std::vector<int>& HSizes, std::vector<int>& VSizes);

bool find_camera(cv::VideoCapture& vsource)
{
    // iterate over all available webcam sources until we find one that opens
    for (int i = 0; i < CAMERA_SLOTS; ++i)
    {
        vsource = cv::VideoCapture(i);
        if (vsource.isOpened()) return true;
    }
    return false;
}

std::vector<CaptureMode> get_supported_video_modes(cv::VideoCapture cap)
{
    static std::vector<int> HSizes; //horizontal resolutions
    static std::vector<int> VSizes; //vertical resolutions

    if (HSizes.empty()) load_resolutions(HSizes, VSizes);

    std::vector<CaptureMode> modes;

    CaptureMode newmode;
    newmode.height = 0;
    newmode.width = 0;

    for (int i = 0; i < HSizes.size(); ++i)
    {
        while (newmode.width > HSizes[i] && newmode.height > VSizes[i]) ++i;    //skip resolutions that are smaller (in both dimensions) than the current one
        if (i > HSizes.size()) break;
        
        cap.set(cv::CAP_PROP_FRAME_WIDTH, HSizes[i]);        
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, VSizes[i]);
                
        if (newmode.width != cap.get(cv::CAP_PROP_FRAME_WIDTH) || newmode.height != cap.get(cv::CAP_PROP_FRAME_HEIGHT))   //if resolution has changed then we may add a new capture mode
        {            
            newmode.width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
            newmode.height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
            newmode.frameRate = cap.get(cv::CAP_PROP_FPS);

            modes.push_back(newmode);
            std::cout << "added capture mode: " << newmode.width << "x" << newmode.height << "@" << newmode.frameRate << "fps\n";            
        }
    }
    return modes;
}

void load_resolutions(std::vector<int>& HSizes, std::vector<int>& VSizes)
{
    std::ifstream ifs;
    ifs.open(resolution_list_file);

    if (!ifs.is_open()) {std::cerr << "could not open resolution list file"; return;}

    int htemp;
    int vtemp;

    while (ifs >> htemp >> vtemp)
    {
        HSizes.push_back(htemp);
        VSizes.push_back(vtemp);

        if (!ifs) break;        
    }

    ifs.close();
}
