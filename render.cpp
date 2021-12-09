/*
 * MMAL Video render example app
 *
 * Copyright © 2017 Raspberry Pi (Trading) Ltd.
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <interface/mmal/mmal.h>
#include <interface/mmal/util/mmal_util.h>
#include <interface/mmal/util/mmal_connection.h>
#include <interface/mmal/util/mmal_util_params.h>

#include <sys/time.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

#include "v4l2.hpp"

using namespace cv;

#define ENCODING MMAL_ENCODING_RGB24
// #define ENCODING 0x59455247
// #define ENCODING MMAL_ENCODING_YUV10_COL
#define WIDTH 1280
#define HEIGHT 800

#define MAX_ENCODINGS_NUM 25
typedef struct
{
    MMAL_PARAMETER_HEADER_T header;
    MMAL_FOURCC_T encodings[MAX_ENCODINGS_NUM];
} MMAL_SUPPORTED_ENCODINGS_T;

static timeval t0;
static timeval t1;
static char text[255];

int print_supported_formats(MMAL_PORT_T *port)
{
    MMAL_STATUS_T ret;

    MMAL_SUPPORTED_ENCODINGS_T sup_encodings = {{MMAL_PARAMETER_SUPPORTED_ENCODINGS, sizeof(sup_encodings)}, {0}};
    ret = mmal_port_parameter_get(port, &sup_encodings.header);
    if (ret == MMAL_SUCCESS || ret == MMAL_ENOSPC)
    {
        //Allow ENOSPC error and hope that the desired formats are in the first
        //MAX_ENCODINGS_NUM entries.
        int i;
        int num_encodings = (sup_encodings.header.size - sizeof(sup_encodings.header)) /
                            sizeof(sup_encodings.encodings[0]);
        if (num_encodings > MAX_ENCODINGS_NUM)
            num_encodings = MAX_ENCODINGS_NUM;
        for (i = 0; i < num_encodings; i++)
        {
            printf("%u: %4.4s\n", i, (char *)&sup_encodings.encodings[i]);
        }
    }
    return 0;
}

static void callback_vr_input(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
    (void)port;
    mmal_buffer_header_release(buffer);
}

int main()
{
    MMAL_COMPONENT_T *render = NULL;
    MMAL_PORT_T *input;
    MMAL_POOL_T *pool;
    MMAL_BUFFER_HEADER_T *buffer;

    mmal_component_create("vc.ril.video_render", &render);
    input = render->input[0];

    print_supported_formats(input);

    input->format->encoding = ENCODING;
    input->format->es->video.width = VCOS_ALIGN_UP(WIDTH, 32);
    input->format->es->video.height = VCOS_ALIGN_UP(HEIGHT, 16);
    input->format->es->video.crop.x = 0;
    input->format->es->video.crop.y = 0;
    input->format->es->video.crop.width = WIDTH;
    input->format->es->video.crop.height = HEIGHT;
    input->buffer_num = 3;

    mmal_port_format_commit(input);

    mmal_component_enable(render);

    mmal_port_parameter_set_boolean(input, MMAL_PARAMETER_ZERO_COPY, MMAL_TRUE);

    input->buffer_size = input->buffer_size_recommended;
    // input->buffer_size = WIDTH * HEIGHT;
    input->buffer_num = input->buffer_num_recommended;
    if (input->buffer_num < 2)
        input->buffer_num = 2;
    pool = mmal_port_pool_create(input, input->buffer_num, input->buffer_size);

    if (!pool)
    {
        printf("Oops, ,pool alloc failed\n");
        return -1;
    }

    {
        MMAL_DISPLAYREGION_T param;
        param.hdr.id = MMAL_PARAMETER_DISPLAYREGION;
        param.hdr.size = sizeof(MMAL_DISPLAYREGION_T);

        param.set = MMAL_DISPLAY_SET_LAYER;
        param.layer = 128; //On top of most things

        param.set |= MMAL_DISPLAY_SET_ALPHA;
        param.alpha = 255; //0 = transparent, 255 = opaque

        param.set |= (MMAL_DISPLAY_SET_DEST_RECT | MMAL_DISPLAY_SET_FULLSCREEN);
        param.fullscreen = 0;
        param.dest_rect.x = 0;
        param.dest_rect.y = 100;
        param.dest_rect.width = WIDTH;
        param.dest_rect.height = HEIGHT;
        mmal_port_parameter_set(input, &param.hdr);
    }

    mmal_port_enable(input, callback_vr_input);
    /*******************************************************************/
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
    int fps;

    Mat bgr;
    Mat rgb24;

    start(handle);

    for (;;)
    {
        data = read(handle);
        Mat frame(HEIGHT, WIDTH, CV_8UC1, (unsigned char *)data.start);

        // flip(frame, img, 0);
        // resize(img, dst, dst.size(), 0, 0, INTER_LINEAR);
        cvtColor(frame, rgb24, CV_GRAY2RGB);

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

        putText(rgb24, text, cv::Point(10, 30), FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(240, 100, 100), 1); //FONT_HERSHEY_SIMPLEX

        /**************************************************/
        buffer = mmal_queue_wait(pool->queue);

        // Write something into the buffer.
        // memset(buffer->data, (i << 4) & 0xff, buffer->alloc_size);
        // memset(buffer->data, i, buffer->alloc_size / 2);
        // memset(buffer->data + buffer->alloc_size / 2, 0xff - i, buffer->alloc_size / 2);
        memcpy(buffer->data, rgb24.data, buffer->alloc_size);

        buffer->length = buffer->alloc_size;
        mmal_port_send_buffer(input, buffer);

        /**************************************************/
        // imshow("image", rgb24);
        // waitKey(1);
    }

    /*******************************************************************/
    stop(handle);
    close(handle);

    mmal_port_disable(input);
    mmal_component_destroy(render);

    return 0;
}
