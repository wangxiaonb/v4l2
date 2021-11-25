#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <fcntl.h>

// #include <Python.h>

u_int8_t imagedata[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};

extern "C" int add(int a, int b)
{
    return a + b;
}

extern "C" void *read_frame()
{
    return imagedata;
}

u_int8_t image[640 * 400];

int main()
{
    int ret;

    //打开设备文件
    int fd;
    fd = open("/dev/video0", O_RDWR);
    if (fd == -1)
    {
        perror("Opening Video device");
        return 1;
    }

    //查询设备信息
    struct v4l2_capability cap;
    ioctl(fd, VIDIOC_QUERYCAP, &cap);
    printf("Driver Name:%s\nCard Name:%s\nBus info:%s\nDriver Version:%u.%u.%u\n", cap.driver, cap.card, cap.bus_info, cap.capabilities);

    //查询设备支持哪种格式
    struct v4l2_fmtdesc fmtdesc;
    fmtdesc.index = 0;
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    printf("Support format:\n");
    while (ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc) != -1)
    {
        printf("\t%d.%s\n", fmtdesc.index + 1, fmtdesc.description);
        fmtdesc.index++;
    }

    //设置视频格式
    struct v4l2_format fmt = {0};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = 640;
    fmt.fmt.pix.height = 400;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_GREY;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;
    ret = ioctl(fd, VIDIOC_S_FMT, &fmt);
    if (ret == -1)
    {
        perror("VIDIOC_S_FMT");
        return 1;
    }

    //申请内核态缓冲
    struct v4l2_requestbuffers reqbuf = {0};
    reqbuf.count = 1;
    reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    reqbuf.memory = V4L2_MEMORY_MMAP;
    ret = ioctl(fd, VIDIOC_REQBUFS, &reqbuf);
    if (ret == -1)
    {
        perror("VIDIOC_REQBUFS");
        return 1;
    }

    //查询内核缓冲
    struct v4l2_buffer buf;
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;
    ret = ioctl(fd, VIDIOC_QUERYBUF, &buf);
    if (ret == -1)
    {
        perror("VIDIOC_QUERYBUF");
        return 1;
    }

    //把内核空间分配好的缓冲映射到用户空间
    void *buffer;
    buffer = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
    if (buffer == MAP_FAILED)
    {
        perror("mmap");
        return -1;
    }

    //添加到采集队列
    ret = ioctl(fd, VIDIOC_QBUF, &buf);
    if (ret == -1)
    {
        perror("VIDIOC_QBUF");
        return -1;
    }

    //启动视频采集
    ret = ioctl(fd, VIDIOC_STREAMON, &buf.type);
    if (ret == -1)
    {
        perror("Start Capture");
        return 1;
    }

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    struct timeval tv = {0};
    tv.tv_sec = 1;
    ret = select(fd + 1, &fds, NULL, NULL, &tv);
    if (ret == -1)
    {
        perror("Waiting for Frame");
        return 1;
    }

    ret = ioctl(fd, VIDIOC_DQBUF, buf);

    memcpy(image, buffer, buf.length);

    printf("DATA:\n");
    for (int i = 0; i < 1000; i++)
    {
        printf("%02x ", image[i]);
    }

    //ioctl(fd,VIDIOC_QBUF,&buf);//再次采集

    //停止采集
    ioctl(fd, VIDIOC_STREAMOFF, &buf.type);

    while (1)
    {
        //开始捕获图像
        ret = ioctl(fd, VIDIOC_STREAMON, &buf.type);

        //取出图像数据
        ret = ioctl(fd, VIDIOC_DQBUF, buf);

        //process frame
        memcpy(image, buffer, buf.length);
        printf("DATA:\n");
        for (int i = 0; i < 1000; i++)
        {
            printf("%02x ", image[i]);
        }

        //告知buf可以使用
        ret = ioctl(fd, VIDIOC_QBUF, &buf);
    }

    //资源回收
    munmap(buffer, buf.length);
    close(fd);

    // CvMat cvmat = cvMat(480, 640, CV_8UC3, (void *)buffer);
    // IplImage *img;
    // img = cvDecodeImage(&cvmat, 1);

    // if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf))
    // {
    //     perror("Retrieving Frame");
    //     return 1;
    // }

    // //监测视频采集是否完成
    // int select(int nfds,          //最大文件描述符加1
    //            fd_set *readfds,   //?
    //            fd_set *writefds,  //0
    //            fd_set *exceptfds, //0
    //            struct timeval *timeout);

    // 10.从队列中取出缓冲 ioctl(fd, VIDIOC_DQBUF, &buf);
    // 11.处理图像 process_image(ub.start, ub.len);
    // 12.停止 / 再次采集
    //               ioctl(fd, VIDIOC_STREAMOFF, &type);
    // / ioctl(fd, VIDIOC_QBUF, &buf);

    // 13.资源回收 munmap(ub.start, ub.len);
    // close(fd);
}
