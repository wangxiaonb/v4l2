#include <sys/time.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

#include "v4l2.hpp"

// #define WIDTH 640
// #define HEIGHT 400

#define WIDTH 1280
#define HEIGHT 800

using namespace cv;

static timeval t0;
static timeval t1;
static char text[255];

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    struct buffer data;
    void *handle;

    // handle = open("/dev/video0");
    handle = open("/dev/video0", WIDTH, HEIGHT, "GREY");

    setcontrol(handle, V4L2_CID_EXPOSURE, 800);     // default:800
    setcontrol(handle, V4L2_CID_ANALOGUE_GAIN, 60); // default:16

    setcontrol(handle, V4L2_CID_VBLANK, 200);

    Mat img = Mat::zeros(HEIGHT, WIDTH, CV_8U);
    // Mat dst = Mat::zeros(480, 768, CV_8U);
    gettimeofday(&t0, NULL);
    float t;
    int fps_count = 0;
    int fps;

    start(handle);

    for (;;)
    {
        data = read(handle);
        Mat frame(HEIGHT, WIDTH, CV_8UC1, (unsigned char *)data.start);

        // flip(frame, img, 0);
        // resize(img, dst, dst.size(), 0, 0, INTER_LINEAR);

        fps_count += 1;
        if (fps_count >= 30)
        {
            gettimeofday(&t1, NULL);
            t = (t1.tv_sec - t0.tv_sec) * 1000 + (t1.tv_usec - t0.tv_usec) / 1000;
            fps = (int)(fps_count * 1000 / t);
            sprintf(text, "FPS: %d", fps);
            printf("FPS: %d\n", fps);
            fps_count = 0;
            gettimeofday(&t0, NULL);
        }

        putText(frame, text, cv::Point(10, 30), FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(127, 127, 127), 1); //FONT_HERSHEY_SIMPLEX
        imshow("image", frame);

        // putText(dst, text, cv::Point(10, 30), FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(127, 127, 127), 1); //FONT_HERSHEY_SIMPLEX
        // imshow("image", dst);
        waitKey(1);
    }

    stop(handle);
    close(handle);

    fprintf(stderr, "\n");

    return 0;
}