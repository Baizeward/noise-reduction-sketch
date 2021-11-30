#include "noise_reduction.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>

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

void set_capture_mode(cv::VideoCapture& cap, CaptureMode mode)
{
    cap.set(cv::CAP_PROP_FRAME_WIDTH, mode.width);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, mode.height);
    cap.set(cv::CAP_PROP_FPS, mode.frameRate);
    return;
}

cv::Mat vote_merge_frames(std::vector<cv::Mat>& frames)
{
    cv::Mat merged = frames[0].clone();
    int votes[frames.size()];
    int winner;
    cv::Vec3b p1;
    int oneper = frames[0].cols / 100;
    

    for (int i = 0, width = frames[0].cols; i < width; ++i)
    {
        for (int j = 0, height = frames[0].rows; j < height; ++j)
        {
            
            
            
            for (int e = 0; e < p1.channels; ++e)
            {
                if (i == 200) std::cerr << winner;
                winner = 0;
                for (int k = 0; k < frames.size(); ++k)
                {                    
                    if (i == 210 && j == 210) std::cout << frames[k].at<cv::Vec3b>(j, i) << "\n";
                                    
                    for (int l = 0; l < frames.size(); ++l)
                    {
                        if (frames[k].at<cv::Vec3b>(j, i)[e] - frames[l].at<cv::Vec3b>(j, i)[e] < SIMILARITY_THRESHOLD) votes[l]++;
                    }          
                                        
                }
                for (int v = 0; v < frames.size(); ++v) if (votes[v] > votes[winner]) winner = v;
                merged.at<cv::Vec3b>(j, i)[e] = frames[winner].at<cv::Vec3b>(j, i)[e];
                for (int & vote : votes) vote = 0;                
            }            
        }
        if (i > 0)
        {
            if (i % oneper == 0) std::cout << i / oneper - 1 << "%\n";
        }
    }

    return merged;
}

cv::Mat median_merge_frames(std::vector<cv::Mat>& frames)
{
    auto start = std::chrono::steady_clock::now();
    
    cv::Mat merged = frames[0].clone();
    std::vector<int> values(frames.size());
    int p1;
    int oneper = frames[0].cols / 100;
    int middle = (frames.size() / 2) + (frames.size() % 2);
    

    for (int i = 0, width = frames[0].cols; i < width; ++i)
    {
        for (int j = 0, height = frames[0].rows; j < height; ++j)
        {            
                
            for (int k = 0; k < frames.size(); ++k)
            {                    
                values[k] = frames[k].at<uchar>(j, i);                    
            }
            std::sort(values.begin(), values.end());                              
            merged.at<uchar>(j, i) = values[middle];               
                      
               
        }
        if (i > 0)
        {
            
        }
        if (i % oneper == 0) std::cout << i / oneper - 1 << "%\n";
    }

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::cerr << "time taken processing samples: " << elapsed_seconds.count() << "s\n";

    return merged;
}