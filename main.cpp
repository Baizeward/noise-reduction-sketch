#include <iostream>
#include "noise_reduction.hpp"

int main()
{  

    cv::VideoCapture cap;

    if (!find_camera(cap))
    {
        std::cout << "could not find a compatible webcam, closing";
        return 1;
    }

    std::vector<CaptureMode> modes = get_supported_video_modes(cap);

    double framerate = cap.get(cv::CAP_PROP_FPS);
    int frametime = 1000 / framerate - 1; //get approximate frame time in ms    

    cv::Mat frame;
    while (true)
    {
        char c = cv::waitKey(frametime);
        if (c == ESC) break;        
        cap >> frame;
        if (frame.empty()) continue;
        imshow("frame", frame);

    }

    cap.release();
    cv::destroyAllWindows();
    return 0;

}








