#include <iostream>
#include <signal.h>
#include <cuda_runtime.h>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <cuda_runtime.h>
#include <opencv2/cudafilters.hpp>

static cv::VideoCapture *capPtr=NULL;
void my_handler(int s){
           printf("Caught signal %d\n",s);
       if(capPtr)
        capPtr->release();
           exit(1);
}



int main()
{
    /* Install handler for catching Ctrl-C and close camera so that Argus keeps ok */
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = my_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);

    std::cout<<"VC start =============="<<std::endl;

    const char* gst =  "rtspsrc location=rtsp://admin:@cam1/ch0_0.264 protocols=tcp ! rtph264depay ! h264parse ! omxh264dec ! nvvidconv ! video/x-raw(memory:NVMM), width=1280, height=720, format=(string)BGRx ! nvvidconv ! appsink";
    cv::VideoCapture cap(gst, cv::CAP_GSTREAMER);

    unsigned int width  = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    unsigned int height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    unsigned int fps    = cap.get(cv::CAP_PROP_FPS);
    unsigned int pixels = width*height;
    std::cout <<"Frame size : "<<width<<" x "<<height<<", "<<pixels<<" Pixels "<<fps<<" FPS"<<std::endl;

    if(!cap.isOpened()) {
		std::cout<<"Failed to open camera."<<std::endl;
		return (-1);
    }

    std::cout<<"Camera Opened =============="<<std::endl;
    cv::namedWindow("MyCameraPreview", cv::WINDOW_AUTOSIZE);

    std::cout << "Using unified memory" << std::endl;
    void *unified_ptr;
    unsigned int frameByteSize = pixels * 3;
    std::cout << "Before Cuda Call" << std::endl;
    cudaMallocManaged(&unified_ptr, frameByteSize);
    std::cout << "After Cuda Call" << std::endl;
    cv::Mat frame_out(height, width, CV_8UC3, unified_ptr);
    cv::cuda::GpuMat d_frame_out(height, width, CV_8UC3, unified_ptr);

    cv::Ptr< cv::cuda::Filter > filter = cv::cuda::createSobelFilter(CV_8UC3, CV_8UC3, 1, 1, 1, 1, cv::BORDER_DEFAULT);
    cv::Mat frame_in;
    while(1)
    {
        if (!cap.read(frame_in)) {
        std::cout<<"Capture read error"<<std::endl;
        break;
    }
    else  {
    	std::cout << "Before copy to" << std::endl;
    	frame_in.copyTo(frame_out);
    	std::cout << "After copy to" << std::endl;
    	filter->apply(d_frame_out, d_frame_out);

    	std::cout << "After apply to" << std::endl;
        cv::imshow("MyCameraPreview",frame_out);
        if((char)cv::waitKey(1) == (char)27)
            break;
    }
    }

    cap.release();
    return 0;
}
