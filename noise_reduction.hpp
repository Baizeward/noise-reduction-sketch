#pragma once

#include <opencv2/opencv.hpp>

#define resolution_list_file "known_capture_resolutions.csv"

constexpr int FRAME_SAMPLES = 6;    // number of frames to combine into one denoised image. Framerate will be divided by this number
constexpr int CAMERA_SLOTS = 10;    // number of video capture sources that may be connected to PC
const char ESC = 27;

struct CaptureMode
{
    int width{0};
    int height{0};
    double frameRate{0};
};

bool find_camera(cv::VideoCapture& vsource);
std::vector<CaptureMode> get_supported_video_modes(cv::VideoCapture cap);




