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

    int frametime = 1000 / modes[currentmode].frameRate - 1; //get frame time in ms    

    cv::Mat frame;
    
    cv::namedWindow("frame", cv::WINDOW_KEEPRATIO);
	bool medsampling = false;

	cv::Mat medianBuffer[FRAME_SAMPLES];
	int medcount;	//	index for the medianBuffer

    while (true)
    {
        
		char c = cv::waitKey(frametime);

        switch (c){
            case ESC: goto end_program;
            case 'm': 
                if (currentmode < modes.size() - 1) ++currentmode;
                else currentmode = 0;

                set_capture_mode(cap, modes[currentmode]);
                frametime = 1000 / modes[currentmode].frameRate;
                std::cout << "set mode to " << modes[currentmode].width << "x" << modes[currentmode].height << "@" << modes[currentmode].frameRate << "fps\n";
                break;
			case 'c':
				if (!medsampling)
				{
					medsampling = true;
					medcount = 0;
				}
				else medsampling = false;
				break;
        }        
        
        cap >> frame;        
        if (frame.empty()) continue;
		cv::cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
        
		if (medsampling)
		{
			medianBuffer[medcount] = frame;
			++medcount;
			if (medcount == FRAME_SAMPLES) 
			{					
				imshow("denoised", temporal_median(medianBuffer));
				//medsampling = false;	//	Uncomment this to denoise one frame at a time
				medcount = 0;
			}
			else std::cerr << "\rloaded frame " << medcount;
		}

        imshow("frame", frame);

    }

    end_program:
        cap.release();
        cv::destroyAllWindows();
        return 0;

}