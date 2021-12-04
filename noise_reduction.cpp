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

cv::Mat add_denoise_r(cv::VideoCapture& cap, int samples, int frametime) //TODO convert frametime to static int initialised by first instance?
{
	cv::Mat image;
	char c;
	static int level = 0;
	
	if (samples == 1)
	{		
		cap >> image;
		c = cv::waitKey(frametime);
		return image;
	}
	else
	{
		cv::addWeighted(add_denoise_r(cap, samples/2, frametime), 0.5, add_denoise_r(cap, samples - samples/2, frametime), 0.5, 0.0, image);
		c = cv::waitKey(frametime);

		if (samples > level)
		{
			level = samples;
			std::cerr << "level: " << level << "\n";
			imshow("denoised", image);
		}
		return image;
	}
	
}

cv::Mat temporal_median(cv::Mat * medianBuffer)
{
	//accepts a vector of mat as input and returns the per element median. Input images must all be the same dimension and have only one channel of uchar
	auto start = std::chrono::steady_clock::now();
	
	cv::Mat result = medianBuffer[0].clone();		
		
	cv::parallel_for_(cv::Range(0 , medianBuffer[0].rows * medianBuffer[0].cols), [&](const cv::Range& range){
    for(int r = range.start; r<range.end; r++ )
    	{
         	int i = r / medianBuffer[0].cols;
         	int j = r % medianBuffer[0].cols;
			
			uint8_t temp[FRAME_SAMPLES];
			for (int k = 0; k < FRAME_SAMPLES; ++k) temp[k] = medianBuffer[k].at<uchar>(i, j);
			std::sort(temp, temp + FRAME_SAMPLES);
			result.at<uchar>(i, j) = temp[FRAME_SAMPLES / 2];
    	}
	});
	
	// auto end = std::chrono::steady_clock::now();
	// std::chrono::duration<double> elapsed_seconds = end-start;
    // std::cerr << "processed " << FRAME_SAMPLES << " samples in: " << elapsed_seconds.count() << "s\n";

	return result;
}