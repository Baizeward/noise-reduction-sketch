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
    int currentmode = modes.size() - 1;

    int frametime = 1000 / modes[currentmode].frameRate - 1; //get approximate frame time in ms    

    cv::Mat frame;
    cv::Mat vsource;
    
    cv::namedWindow("frame", cv::WINDOW_KEEPRATIO);
    bool sampling = false;
    std::vector<cv::Mat> samples(FRAME_SAMPLES);
    int si = 0; //sample index

    while (true)
    {
        char c = cv::waitKey(frametime);

        switch (c){
            case ESC: goto label;
            case 'm': 
                if (currentmode < modes.size() - 1) ++currentmode;
                else currentmode = 0;

                set_capture_mode(cap, modes[currentmode]);
                frametime = 1000 / modes[currentmode].frameRate;
                std::cout << "set mode to " << modes[currentmode].width << "x" << modes[currentmode].height << "@" << modes[currentmode].frameRate << "fps\n";
                break;
            case 'c': 
                sampling = true; 
                si = 0;
                break; 
        }        
        
        cap >> vsource;
        cv::cvtColor(vsource, frame, cv::COLOR_BGR2GRAY);
        if (frame.empty()) continue;
        if (sampling)
        {
            samples[si] = frame.clone();
            ++si;
            if (si == FRAME_SAMPLES) {imshow("denoised", median_merge_frames(samples)); sampling = false;}
        }

        imshow("frame", frame);

    }

    label:
        cap.release();
        cv::destroyAllWindows();
        return 0;

}








