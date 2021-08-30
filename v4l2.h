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
    ~cv4l2();

    void open_device(void);
    void init_device(void);
    void mainloop(void);
    void stop_capturing(void);
    void start_capturing(void);
    void close_device(void);
    void uninit_device(void);

    struct buffer read_frame();

private:
    void errno_exit(const char *s);
    int xioctl(int fh, int request, void *arg);
    void init_read(unsigned int buffer_size);
    void init_mmap(void);
    void init_userp(unsigned int buffer_size);
    void process_image(const void *p, int size);
    int read_frame_origin(void);

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
    int force_format;
    int frame_count = 100;
};

cv4l2 *open(const char *dev_name);
void close(cv4l2 *handle);
void start(cv4l2 *handle);
void stop(cv4l2 *handle);
struct buffer read(cv4l2 *handle);

int add(int, int);