#include "v4l2.hpp"

#include <sys/time.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

using namespace cv; // 省去函数前面加cv::的必要性

static timeval t0;
static timeval t1;

static timeval brenner_t0;
static timeval brenner_t1;

static char text[255];

#pragma GCC push_options
#pragma GCC optimize("O3")
double brenner(cv::Mat &image)
{
    // assert(image.empty());

    // cv::Mat gray_img;
    // if (image.channels() == 3)
    // {
    //     cv::cvtColor(image, gray_img, CV_BGR2GRAY);
    // }
    // else
    // {
    //     gray_img = image;
    // }

    double result = .0f;
    for (int i = 0; i < image.rows; ++i)
    {
        uchar *data = image.ptr<uchar>(i);
        for (int j = 0; j < image.cols - 2; ++j)
        {
            result += pow(data[j + 2] - data[j], 2);
        }
    }

    return result;
    // return result / image.total();
}
#pragma GCC pop_options

// float brenner(Mat img, int width, int height)
// {
//     // '''
//     // :param img:narray 二维灰度图像
//     // :return: float 图像越清晰越大
//     // '''

//     uchar *ptmp = NULL;
//     float out = 0;
//     for (int x = 0; x < height - 2; x++)
//     {
//         for (int y = 0; y < width; y++)
//         {
//             out += ((int)(img.data[x + 2][y]) - (int)(img.data[x][y])) ^ 2;
//         }
//     }

//     return out;
// }

// #define WIDTH 640
// #define HEIGHT 400
#define WIDTH 640
#define HEIGHT 480

__u8 buffer[WIDTH * HEIGHT];

int main(int argc, char **argv)
{
    __u16 regval;
    struct buffer data;

    // void *handle = open("/dev/video0");
    void *handle = open("/dev/video2", WIDTH, HEIGHT, "YUYV"); //ssjhs GREY, not GRAY!!!!!!!!!!

    // setformat(handle, 1280,800,'GREY');

    getcontrol(handle, V4L2_CID_BRIGHTNESS);
    setcontrol(handle, V4L2_CID_BRIGHTNESS, 240);
    setcontrol(handle, V4L2_CID_GAIN, 32);

    regval = get_sensor_reg(handle, 0x0b);
    printf("get_sensor_reg:%d \r\n", regval);

    set_sensor_reg(handle, 0x0b, 300);
    regval = get_sensor_reg(handle, 0x0b);
    printf("get_sensor_reg:%d \r\n", regval);

    start(handle);

    gettimeofday(&t0, NULL);
    float t;
    int fps_count = 0;
    int fps;
    // double value = 0;
    // float interval;

    Mat img = Mat::zeros(HEIGHT, WIDTH, CV_8U);
    // Mat dst = Mat::zeros(480, 768, CV_8UC1);
    // Mat dst = Mat::zeros(600, 960, CV_8UC1);

    for (;;)
    {
        // snapshot(handle);
        data = read(handle);

        __u8* pointer = (__u8*)data.start;

// #pragma GCC push_options
// #pragma GCC optimize("O1")
        for (unsigned int i = 0; i < sizeof(buffer); i++)
        {
            buffer[i] = pointer[i * 2];
        }
// #pragma GCC pop_options

        Mat frame(HEIGHT, WIDTH, CV_8UC1, (unsigned char *)buffer);

        flip(frame, img, 0);
        // resize(img, dst, dst.size(), 0, 0, INTER_LINEAR);

        fps_count += 1;
        if (fps_count >= 30)
        {
            gettimeofday(&t1, NULL);
            t = (t1.tv_sec - t0.tv_sec) * 1000 + (t1.tv_usec - t0.tv_usec) / 1000;
            fps = (int)(fps_count * 1000 / t);
            sprintf(text, "FPS: %d", fps);
            printf("FPS: %d\n", fps);
            // printf("brenner: %f\n", value);
            // printf("interval: %f\n", interval);
            fps_count = 0;
            gettimeofday(&t0, NULL);
        }

        // gettimeofday(&brenner_t0, NULL);
        // value = brenner(img);
        // gettimeofday(&brenner_t1, NULL);
        // interval = (brenner_t1.tv_sec - brenner_t0.tv_sec) * 1000 + (brenner_t1.tv_usec - brenner_t0.tv_usec) / 1000;

        putText(img, text, cv::Point(10, 30), FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 0), 1); //FONT_HERSHEY_SIMPLEX

        imshow("image", img);
        waitKey(1);
    }

    stop(handle);
    close(handle);

    fprintf(stderr, "\n");

    return 0;
}
