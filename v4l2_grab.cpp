/* V4L2 video picture grabber
Copyright (C) 2009 Mauro Carvalho Chehab <mchehab@infradead.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <libv4l2.h>
#include <pthread.h>
#include <sys/time.h>

#define CLEAR(x) memset(&(x), 0, sizeof(x))

#define LED_PIN 17
#define WIDTH 640
#define HEIGHT 400

extern void led_init(int pin);
extern void led_flash(int udelay);
extern void led_release();

static char image[20][WIDTH * HEIGHT];

struct buffer
{
        void *start;
        size_t length;
};

static void xioctl(int fh, int request, void *arg)
{
        int r;
        do
        {
                r = ioctl(fh, request, arg);
        } while (r == -1 && ((errno == EINTR) || (errno == EAGAIN)));

        if (r == -1)
        {
                fprintf(stderr, "error %d, %s\\n", errno, strerror(errno));
                exit(EXIT_FAILURE);
        }
}

static void set_control(int fd, __u32 id, __s32 value)
{
        int value_old;
        struct v4l2_control ctrl;
        ctrl.id = id;
        if (-1 == ioctl(fd, VIDIOC_G_CTRL, &ctrl))
        {
                perror("VIDIOC_G_CTRL");
        }
        value_old = ctrl.value;
        ctrl.value = value;
        if (-1 == ioctl(fd, VIDIOC_S_CTRL, &ctrl))
        {
                perror("VIDIOC_S_CTRL");
        }
        if (-1 == ioctl(fd, VIDIOC_G_CTRL, &ctrl))
        {
                perror("VIDIOC_G_CTRL");
        }
        printf("Set Control id: %04x, from %d to %d\n", ctrl.id, value_old, ctrl.value);
}

static __s32 get_control(int fd, __u32 id)
{
        struct v4l2_control ctrl;
        ctrl.id = id;
        if (-1 == ioctl(fd, VIDIOC_G_CTRL, &ctrl))
        {
                perror("VIDIOC_G_CTRL");
        }
        printf("Get Control id: %04x, value: %d\n", ctrl.id, ctrl.value);
        return ctrl.value;
}

int flash_time = 1600 * 1;
int exposure_time = 500;
int N_FRAME = 5;
pthread_t tid1;
static void *thread1(void *arg)
{
        // for (;;)
        {
                led_flash(flash_time);
                // usleep(flash_time);
        }
        pthread_exit(0);
}

int main(int argc, char **argv)
{
        printf("argc:%d\n", argc);
        if (argc >= 3)
        {
                flash_time = atoi(argv[1]);
                exposure_time = atoi(argv[2]);
        }
        printf("flash_time:%d\n", flash_time);
        printf("exposure_time:%d\n", exposure_time);

        struct v4l2_format fmt;
        struct v4l2_buffer buf;
        struct v4l2_requestbuffers req;
        enum v4l2_buf_type type;
        fd_set fds;
        struct timeval tv;
        int r, fd = -1;
        unsigned int i, n_buffers;
        const char *dev_name = "/dev/video1";
        char out_name[256];
        FILE *fout;
        struct buffer *buffers;

        led_init(LED_PIN);

        fd = open(dev_name, O_RDWR | O_NONBLOCK, 0);
        if (fd < 0)
        {
                perror("Cannot open device");
                exit(EXIT_FAILURE);
        }

        CLEAR(fmt);
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width = WIDTH;
        fmt.fmt.pix.height = HEIGHT;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_GREY;
        fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
        xioctl(fd, VIDIOC_S_FMT, &fmt);
        if (fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_GREY)
        {
                printf("Libv4l didn't accept RGB24 format. Can't proceed.\\n");
                exit(EXIT_FAILURE);
        }
        if ((fmt.fmt.pix.width != WIDTH) || (fmt.fmt.pix.height != HEIGHT))
                printf("Warning: driver is sending image at %dx%d\\n",
                       fmt.fmt.pix.width, fmt.fmt.pix.height);

        CLEAR(req);
        req.count = 2;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_MMAP;
        xioctl(fd, VIDIOC_REQBUFS, &req);

        buffers = (struct buffer *)calloc(req.count, sizeof(*buffers));
        for (n_buffers = 0; n_buffers < req.count; ++n_buffers)
        {
                CLEAR(buf);

                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;
                buf.index = n_buffers;

                xioctl(fd, VIDIOC_QUERYBUF, &buf);

                buffers[n_buffers].length = buf.length;
                buffers[n_buffers].start = mmap(NULL, buf.length,
                                                PROT_READ | PROT_WRITE, MAP_SHARED,
                                                fd, buf.m.offset);

                if (MAP_FAILED == buffers[n_buffers].start)
                {
                        perror("mmap");
                        exit(EXIT_FAILURE);
                }
        }

        for (i = 0; i < n_buffers; ++i)
        {
                CLEAR(buf);
                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;
                buf.index = i;
                xioctl(fd, VIDIOC_QBUF, &buf);
        }
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        int value = get_control(fd, V4L2_CID_EXPOSURE);
        set_control(fd, V4L2_CID_EXPOSURE, exposure_time);
        set_control(fd, V4L2_CID_ANALOGUE_GAIN, 16); //default:16
        // set_control(fd,V4L2_CID_PIXEL_RATE,20000);//default=200000000 value=2000000
        set_control(fd,V4L2_CID_VBLANK,4000);//min=21 max=32367 step=1 default=21 value=21
        value = get_control(fd,V4L2_CID_HBLANK);//min=816 max=816 step=1 default=816 value=816 flags=read-only

        xioctl(fd, VIDIOC_STREAMON, &type);

        struct timeval tv1;
        struct timeval tv2;
        gettimeofday(&tv1, NULL);

        for (i = 0; i < 20; i++)
        {
                if (i == N_FRAME)
                {
                        // led_flash(3000000);
                        pthread_create(&tid1, NULL, thread1, NULL);
                        // pthread_join(tid1, NULL);
                }
                do
                {
                        FD_ZERO(&fds);
                        FD_SET(fd, &fds);

                        /* Timeout. */
                        tv.tv_sec = 2;
                        tv.tv_usec = 0;

                        r = select(fd + 1, &fds, NULL, NULL, &tv);
                } while ((r == -1 && (errno = EINTR)));
                if (r == -1)
                {
                        perror("select");
                        return errno;
                }

                CLEAR(buf);
                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;
                xioctl(fd, VIDIOC_DQBUF, &buf);

                // if (i == N_FRAME + 6)
                {
                        memcpy(image[i], buffers[buf.index].start, buf.bytesused);
                }

                // if (i == N_FRAME)
                // {
                //         sprintf(out_name, "out%03d.ppm", i);
                //         fout = fopen(out_name, "w");
                //         if (!fout)
                //         {
                //                 perror("Cannot open image");
                //                 exit(EXIT_FAILURE);
                //         }
                //         fprintf(fout, "P5\n%d %d\n255\n",
                //                 fmt.fmt.pix.width, fmt.fmt.pix.height);
                //         fwrite(buffers[buf.index].start, buf.bytesused, 1, fout);
                //         fclose(fout);
                // }

                xioctl(fd, VIDIOC_QBUF, &buf);
        }
        gettimeofday(&tv2, NULL);

        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        xioctl(fd, VIDIOC_STREAMOFF, &type);
        for (i = 0; i < n_buffers; ++i)
                munmap(buffers[i].start, buffers[i].length);
        close(fd);

        led_release();

        for (int i = 0; i < 20; i++)
        {
                sprintf(out_name, "./ppm/out%03d.ppm", i);
                fout = fopen(out_name, "w");
                if (!fout)
                {
                        perror("Cannot open image");
                        exit(EXIT_FAILURE);
                }
                fprintf(fout, "P5\n%d %d\n255\n",
                        fmt.fmt.pix.width, fmt.fmt.pix.height);
                fwrite(image[i], sizeof(image[i]), 1, fout);
                fclose(fout);
        }

        printf("time interval:%dus\r\n", (tv2.tv_sec - tv1.tv_sec) * 1000000 + tv2.tv_usec - tv1.tv_usec);

        pthread_join(tid1, NULL);

        return 0;
}