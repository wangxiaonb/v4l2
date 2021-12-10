#include "mmal.hpp"

#include <sys/time.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

#include "v4l2.hpp"

using namespace cv;

#define WIDTH 1280
#define HEIGHT 800

int main()
{
    cmmal *mmal = new cmmal();

    mmal->open(WIDTH, HEIGHT);
    mmal->set(128, 255, 0, 150, WIDTH, HEIGHT);
    /*******************************************************************/
    timeval t0;
    timeval t1;
    char text[255];
    struct buffer data;
    void *handle;

    handle = open("/dev/video0", WIDTH, HEIGHT, "GREY");

    setcontrol(handle, V4L2_CID_EXPOSURE, 800);     // default:800
    setcontrol(handle, V4L2_CID_ANALOGUE_GAIN, 24); // default:16

    setcontrol(handle, V4L2_CID_VBLANK, 3500);

    // Mat img = Mat::zeros(HEIGHT, WIDTH, CV_8U);
    // Mat dst = Mat::zeros(480, 768, CV_8U);
    gettimeofday(&t0, NULL);
    float t;
    int fps_count = 0;
    int fps = 0;
    sprintf(text, "FPS:      ");

    Mat bgr;
    Mat bgr24;

    start(handle);
    /*******************************************************************/
    int y = 0;
    for (;;)
    {
        mmal->set(128, 255, 0, y*2, WIDTH, HEIGHT);
        if (y++ > 125)
        {
            y = 0;
        }
        /**************************************************/
        data = read(handle);
        Mat frame(HEIGHT, WIDTH, CV_8UC1, (unsigned char *)data.start);

        // flip(frame, img, 0);
        // resize(img, dst, dst.size(), 0, 0, INTER_LINEAR);
        cvtColor(frame, bgr24, CV_GRAY2BGR);

        fps_count += 1;
        if (fps_count >= 30)
        {
            gettimeofday(&t1, NULL);
            t = (t1.tv_sec - t0.tv_sec) * 1000 + (t1.tv_usec - t0.tv_usec) / 1000;
            fps = (int)(fps_count * 1000 / t);
            memset(text, 0, sizeof(text));
            sprintf(text, "FPS: %d", fps);
            printf("FPS: %d\n", fps);
            fps_count = 0;
            gettimeofday(&t0, NULL);
        }

        putText(bgr24, text, cv::Point(10, 30), FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(100, 100, 240), 2); //FONT_HERSHEY_SIMPLEX
        /**************************************************/
        mmal->show(bgr24.data);
    }

    /*******************************************************************/
    stop(handle);
    close(handle);

    mmal->close();

    return 0;
}
