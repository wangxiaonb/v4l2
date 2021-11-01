
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <linux/uvcvideo.h>
#include <linux/usb/video.h>
#include <errno.h>
#include <string.h>

#define CLEAR(x) memset(&(x), 0, sizeof(x))

#define UVC_EU1_ID 0x06
#define EU1_TEST_CMD 0x3b
#define EU1_TEST_CMD_LEN 0x01

int xioctl(int fh, int request, void *arg)
{
    int r;

    do
    {
        r = ioctl(fh, request, arg);
    } while (-1 == r && EINTR == errno);

    return r;
}

__attribute((aligned(8))) unsigned char buf[8];

int main(int argc, char **argv)
{
    int ret;
    int fd = open("/dev/video2", O_RDWR);
    if (fd <= 0)
    {
        printf("open failed!\n");
        return -1;
    }

    struct uvc_xu_control_query xctrl;
    xctrl.unit = 3;
    xctrl.selector = 1;
    xctrl.query = UVC_GET_CUR;
    xctrl.size = 4;
    xctrl.data = buf;

    buf[1] = 0x46;

    buf[0] = 0;
    xctrl.query = UVC_SET_CUR;
    ret = xioctl(fd, UVCIOC_CTRL_QUERY, &xctrl);
    if (ret < 0)
    {
        printf("ioctl(UVCIOC_CTRL_QUERY)");
        printf("%s\n", strerror(errno));
    }
    else
    {
        printf("UVC_SET_CUR: %08x\r\n", *(u_int32_t *)(&buf));
    }

    xctrl.query = UVC_GET_CUR;
    ret = xioctl(fd, UVCIOC_CTRL_QUERY, &xctrl);
    if (ret < 0)
    {
        printf("ioctl(UVCIOC_CTRL_QUERY)");
        printf("%s\n", strerror(errno));
    }
    else
    {
        printf("UVC_GET_CUR: %08x\r\n", *(u_int32_t *)(&buf));
    }

    //////////////////////////////////////////////////////////////////
    buf[0] = 1;
    buf[2] = 0xAB;
    buf[3] = 0xCD;
    xctrl.query = UVC_SET_CUR;
    ret = xioctl(fd, UVCIOC_CTRL_QUERY, &xctrl);
    if (ret < 0)
    {
        printf("ioctl(UVCIOC_CTRL_QUERY)");
        printf("%s\n", strerror(errno));
    }
    else
    {
        printf("UVC_SET_CUR: %08x\r\n", *(u_int32_t *)(&buf));
    }

    xctrl.query = UVC_GET_CUR;
    ret = xioctl(fd, UVCIOC_CTRL_QUERY, &xctrl);
    if (ret < 0)
    {
        printf("ioctl(UVCIOC_CTRL_QUERY)");
        printf("%s\n", strerror(errno));
    }
    else
    {
        printf("UVC_GET_CUR: %08x\r\n", *(u_int32_t *)(&buf));
    }

    return 0;
}
