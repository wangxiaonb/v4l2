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

class cv4l2
{
public:
    cv4l2(const char *dev);

    void open_device(void);
    void init_device(void);
    void mainloop(void);
    void stop_capturing(void);
    void start_capturing(void);
    void close_device(void);
    void uninit_device(void);
    

private:
    void errno_exit(const char *s);
    int xioctl(int fh, int request, void *arg);
    void process_image(const void *p, int size);
    int read_frame(void);
    void init_read(unsigned int buffer_size);
    void init_mmap(void);
    void init_userp(unsigned int buffer_size);

private:
    enum io_method
    {
        IO_METHOD_READ = 1,
        IO_METHOD_MMAP = 2,
        IO_METHOD_USERPTR = 3,
    };

    struct buffer
    {
        void *start;
        size_t length;
    };

    char dev_name[255];
    enum io_method io = IO_METHOD_MMAP;
    int fd = -1;
    buffer *buffers;
    unsigned int n_buffers;
    int out_buf;
    int force_format;
    int frame_count = 200;
};