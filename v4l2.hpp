#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <getopt.h> /* getopt_long() */

#include <fcntl.h> /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <linux/videodev2.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp> 

#define CLEAR(x) memset(&(x), 0, sizeof(x))

struct buffer
{
    void *start;
    size_t length;
};

class cv4l2
{
public:
    cv4l2(const char *dev);
    cv4l2(const char *dev, int width, int height, const char *color);
    ~cv4l2();

    void open_device(void);
    void init_device(void);
    void start_capturing(void);
    struct buffer read_frame();
    void stop_capturing(void);
    void uninit_device(void);
    void close_device(void);

    void set_format(int width, int height, const char *color);
    void set_control(__u32 id, __s32 value);
    __s32 get_control(__u32 id);

private:
    void errno_exit(const char *s);
    int xioctl(int fh, int request, void *arg);
    void init_read(unsigned int buffer_size);
    void init_mmap(void);
    void init_userp(unsigned int buffer_size);
    struct buffer read_frame_real();

private:
    enum io_method
    {
        IO_METHOD_READ = 1,
        IO_METHOD_MMAP = 2,
        IO_METHOD_USERPTR = 3,
    };

    char dev_name[255];
    enum io_method io = IO_METHOD_MMAP;
    int fd = -1;
    buffer *buffers;
    unsigned int n_buffers;
    int out_buf;
    int force_format = 1;
    // int width = 1280;
    // int height = 800;
    int width = 640;
    int height = 480;
    // __u32 color = 0x59455247;
    __u32 color = V4L2_PIX_FMT_YUYV;
    int frame_count = 100;
};

void *open(const char *dev_name);
void *open(const char *dev_name, int width, int height, const char *color);
void close(void *handle);
void start(void *handle);
void stop(void *handle);
struct buffer read(void *handle);

void setformat(void *handle, int width, int height, const char *color);
void setcontrol(void *handle, __u32 id, __s32 value);
__s32 getcontrol(void *handle, __u32 id);
