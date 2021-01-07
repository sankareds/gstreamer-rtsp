#include <iostream>
#include <signal.h>

#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/core.hpp>

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

    const char* gst =  "rtspsrc location=rtsp://admin:@cam1/ch0_0.264 protocols=tcp drop_on_latency=true tcp_buffer_size=1572864 do_retransmission=false latency=1000 ! queue max_size_time=5000000000 max-size-bytes=0 max-size-buffers=0 leaky=1 ! rtph264depay ! h264parse ! omxh264dec ! nvvidconv ! video/x-raw, width=1280, height=720, format=BGRx ! videoconvert ! appsink name=appsink max-buffers=5 drop=1";
    cv::VideoCapture cap(gst, cv::CAP_GSTREAMER);
    if(!cap.isOpened()) {
    std::cout<<"Failed to open camera."<<std::endl;
    return (-1);
    }

    std::cout<<"Camera Opened =============="<<std::endl;
    cv::namedWindow("MyCameraPreview", cv::WINDOW_AUTOSIZE);


    cv::Mat frame_in;
    while(1)
    {
        if (!cap.read(frame_in)) {
        std::cout<<"Capture read error"<<std::endl;
        break;
    }
    else  {
    	cap >> frame_in;
        cv::imshow("MyCameraPreview",frame_in);
        if((char)cv::waitKey(1) == (char)27)
            break;
    }
    }

    cap.release();
    return 0;
}
