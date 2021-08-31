/*
*  V4L2 video capture example
*
*  This program can be used and distributed without restrictions.
*
*      This program is provided with the V4L2 API
* see https://linuxtv.org/docs.php for more information
*/

#include "v4l2.hpp"

/***************************************************************************/
cv4l2 ::cv4l2(const char *dev)
{
    strcpy(dev_name, dev);
}

cv4l2::cv4l2(const char *dev, int width, int height, const char *color)
{
    strcpy(dev_name, dev);
    this->width = width;
    this->height = height;
    this->color = v4l2_fourcc(color[0], color[1], color[2], color[3]);
}

cv4l2::~cv4l2()
{
}

/******************************Private*****************************************/
void cv4l2::errno_exit(const char *s)
{
    fprintf(stderr, "%s error %d, %s\\n", s, errno, strerror(errno));
    exit(EXIT_FAILURE);
}

int cv4l2::xioctl(int fh, int request, void *arg)
{
    int r;

    do
    {
        r = ioctl(fh, request, arg);
    } while (-1 == r && EINTR == errno);

    return r;
}

void cv4l2::init_read(unsigned int buffer_size)
{
    buffers = (struct buffer *)calloc(1, sizeof(*buffers));

    if (!buffers)
    {
        fprintf(stderr, "Out of memory\\n");
        exit(EXIT_FAILURE);
    }

    buffers[0].length = buffer_size;
    buffers[0].start = malloc(buffer_size);

    if (!buffers[0].start)
    {
        fprintf(stderr, "Out of memory\\n");
        exit(EXIT_FAILURE);
    }
}

void cv4l2::init_mmap(void)
{
    struct v4l2_requestbuffers req;

    CLEAR(req);

    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req))
    {
        if (EINVAL == errno)
        {
            fprintf(stderr, "%s does not support "
                            "memory mappingn",
                    dev_name);
            exit(EXIT_FAILURE);
        }
        else
        {
            errno_exit("VIDIOC_REQBUFS");
        }
    }

    if (req.count < 2)
    {
        fprintf(stderr, "Insufficient buffer memory on %s\\n",
                dev_name);
        exit(EXIT_FAILURE);
    }

    buffers = (struct buffer *)calloc(req.count, sizeof(*buffers));

    if (!buffers)
    {
        fprintf(stderr, "Out of memory\\n");
        exit(EXIT_FAILURE);
    }

    for (n_buffers = 0; n_buffers < req.count; ++n_buffers)
    {
        struct v4l2_buffer buf;

        CLEAR(buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = n_buffers;

        if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
            errno_exit("VIDIOC_QUERYBUF");

        buffers[n_buffers].length = buf.length;
        buffers[n_buffers].start =
            mmap(NULL /* start anywhere */,
                 buf.length,
                 PROT_READ | PROT_WRITE /* required */,
                 MAP_SHARED /* recommended */,
                 fd, buf.m.offset);

        if (MAP_FAILED == buffers[n_buffers].start)
            errno_exit("mmap");
    }
}

void cv4l2::init_userp(unsigned int buffer_size)
{
    struct v4l2_requestbuffers req;

    CLEAR(req);

    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_USERPTR;

    if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req))
    {
        if (EINVAL == errno)
        {
            fprintf(stderr, "%s does not support "
                            "user pointer i/on",
                    dev_name);
            exit(EXIT_FAILURE);
        }
        else
        {
            errno_exit("VIDIOC_REQBUFS");
        }
    }

    buffers = (struct buffer *)calloc(4, sizeof(*buffers));

    if (!buffers)
    {
        fprintf(stderr, "Out of memory\\n");
        exit(EXIT_FAILURE);
    }

    for (n_buffers = 0; n_buffers < 4; ++n_buffers)
    {
        buffers[n_buffers].length = buffer_size;
        buffers[n_buffers].start = malloc(buffer_size);

        if (!buffers[n_buffers].start)
        {
            fprintf(stderr, "Out of memory\\n");
            exit(EXIT_FAILURE);
        }
    }
}

struct buffer cv4l2::read_frame_real()
{
    struct buffer frame = {NULL, 0};

    struct v4l2_buffer buf;
    unsigned int i;

    switch (io)
    {
    case IO_METHOD_READ:
        if (-1 == read(fd, buffers[0].start, buffers[0].length))
        {
            switch (errno)
            {
            case EAGAIN:
                return frame;
                // return 0;

            case EIO:
                /* Could ignore EIO, see spec. */

                /* fall through */

            default:
                errno_exit("read");
            }
        }

        frame = {buffers[0].start, buffers[0].length};
        // process_image(buffers[0].start, buffers[0].length);
        break;

    case IO_METHOD_MMAP:
        CLEAR(buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf))
        {
            switch (errno)
            {
            case EAGAIN:
                return frame;
                // return 0;

            case EIO:
                /* Could ignore EIO, see spec. */

                /* fall through */

            default:
                errno_exit("VIDIOC_DQBUF");
            }
        }

        assert(buf.index < n_buffers);

        // process_image(buffers[buf.index].start, buf.bytesused);
        frame = {buffers[buf.index].start, buf.bytesused};

        if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
            errno_exit("VIDIOC_QBUF");
        break;

    case IO_METHOD_USERPTR:
        CLEAR(buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_USERPTR;

        if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf))
        {
            switch (errno)
            {
            case EAGAIN:
                return frame;
                // return 0;

            case EIO:
                /* Could ignore EIO, see spec. */

                /* fall through */

            default:
                errno_exit("VIDIOC_DQBUF");
            }
        }

        for (i = 0; i < n_buffers; ++i)
            if (buf.m.userptr == (unsigned long)buffers[i].start && buf.length == buffers[i].length)
                break;

        assert(i < n_buffers);

        frame = {(void *)buf.m.userptr, buf.bytesused};
        // process_image((void *)buf.m.userptr, buf.bytesused);

        if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
            errno_exit("VIDIOC_QBUF");
        break;
    }

    return frame;
}

/*******************************Public*******************************************/

void cv4l2::open_device(void)
{
    struct stat st;

    if (-1 == stat(dev_name, &st))
    {
        fprintf(stderr, "Cannot identify '%s': %d, %s\\n",
                dev_name, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (!S_ISCHR(st.st_mode))
    {
        fprintf(stderr, "%s is no device\n", dev_name);
        exit(EXIT_FAILURE);
    }

    fd = open(dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);

    if (-1 == fd)
    {
        fprintf(stderr, "Cannot open '%s': %d, %s\\n",
                dev_name, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void cv4l2::init_device(void)
{
    struct v4l2_capability cap;
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    struct v4l2_format fmt;
    unsigned int min;

    if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap))
    {
        if (EINVAL == errno)
        {
            fprintf(stderr, "%s is no V4L2 device\\n",
                    dev_name);
            exit(EXIT_FAILURE);
        }
        else
        {
            errno_exit("VIDIOC_QUERYCAP");
        }
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    {
        fprintf(stderr, "%s is no video capture device\\n",
                dev_name);
        exit(EXIT_FAILURE);
    }

    switch (io)
    {
    case IO_METHOD_READ:
        if (!(cap.capabilities & V4L2_CAP_READWRITE))
        {
            fprintf(stderr, "%s does not support read i/o\\n",
                    dev_name);
            exit(EXIT_FAILURE);
        }
        break;

    case IO_METHOD_MMAP:
    case IO_METHOD_USERPTR:
        if (!(cap.capabilities & V4L2_CAP_STREAMING))
        {
            fprintf(stderr, "%s does not support streaming i/o\\n",
                    dev_name);
            exit(EXIT_FAILURE);
        }
        break;
    }

    /* Select video input, video standard and tune here. */

    CLEAR(cropcap);

    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (0 == xioctl(fd, VIDIOC_CROPCAP, &cropcap))
    {
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        crop.c = cropcap.defrect; /* reset to default */

        if (-1 == xioctl(fd, VIDIOC_S_CROP, &crop))
        {
            switch (errno)
            {
            case EINVAL:
                /* Cropping not supported. */
                break;
            default:
                /* Errors ignored. */
                break;
            }
        }
    }
    else
    {
        /* Errors ignored. */
    }

    CLEAR(fmt);

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (force_format)
    {
        // fmt.fmt.pix.width = 640;
        // fmt.fmt.pix.height = 480;
        // fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
        fmt.fmt.pix.width = width;
        fmt.fmt.pix.height = height;
        fmt.fmt.pix.pixelformat = color; //V4L2_PIX_FMT_GREY;
        fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

        if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
            errno_exit("VIDIOC_S_FMT");

        /* Note VIDIOC_S_FMT may change width and height. */
    }
    else
    {
        /* Preserve original settings as set by v4l2-ctl for example */
        if (-1 == xioctl(fd, VIDIOC_G_FMT, &fmt))
            errno_exit("VIDIOC_G_FMT");
    }

    /* Buggy driver paranoia. */
    min = fmt.fmt.pix.width * 2;
    if (fmt.fmt.pix.bytesperline < min)
        fmt.fmt.pix.bytesperline = min;
    min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
    if (fmt.fmt.pix.sizeimage < min)
        fmt.fmt.pix.sizeimage = min;

    switch (io)
    {
    case IO_METHOD_READ:
        init_read(fmt.fmt.pix.sizeimage);
        break;

    case IO_METHOD_MMAP:
        init_mmap();
        break;

    case IO_METHOD_USERPTR:
        init_userp(fmt.fmt.pix.sizeimage);
        break;
    }
}

struct buffer cv4l2::read_frame()
{
    struct buffer frame;
    for (;;)
    {
        fd_set fds;
        struct timeval tv;
        int r;

        FD_ZERO(&fds);
        FD_SET(fd, &fds);

        /* Timeout. */
        tv.tv_sec = 2;
        tv.tv_usec = 0;

        r = select(fd + 1, &fds, NULL, NULL, &tv);

        if (-1 == r)
        {
            if (EINTR == errno)
                continue;
            errno_exit("select");
        }

        if (0 == r)
        {
            fprintf(stderr, "select timeout\n");
            exit(EXIT_FAILURE);
        }

        frame = read_frame_real();
        if (frame.length > 0)
            return frame;
        /* EAGAIN - continue select loop. */
    }
    return {NULL, 0};
}

void cv4l2::start_capturing(void)
{
    unsigned int i;
    enum v4l2_buf_type type;

    switch (io)
    {
    case IO_METHOD_READ:
        /* Nothing to do. */
        break;

    case IO_METHOD_MMAP:
        for (i = 0; i < n_buffers; ++i)
        {
            struct v4l2_buffer buf;

            CLEAR(buf);
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index = i;

            if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
                errno_exit("VIDIOC_QBUF");
        }
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
            errno_exit("VIDIOC_STREAMON");
        break;

    case IO_METHOD_USERPTR:
        for (i = 0; i < n_buffers; ++i)
        {
            struct v4l2_buffer buf;

            CLEAR(buf);
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_USERPTR;
            buf.index = i;
            buf.m.userptr = (unsigned long)buffers[i].start;
            buf.length = buffers[i].length;

            if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
                errno_exit("VIDIOC_QBUF");
        }
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
            errno_exit("VIDIOC_STREAMON");
        break;
    }
}

void cv4l2::stop_capturing(void)
{
    enum v4l2_buf_type type;

    switch (io)
    {
    case IO_METHOD_READ:
        /* Nothing to do. */
        break;

    case IO_METHOD_MMAP:
    case IO_METHOD_USERPTR:
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &type))
            errno_exit("VIDIOC_STREAMOFF");
        break;
    }
}

void cv4l2::uninit_device(void)
{
    unsigned int i;

    switch (io)
    {
    case IO_METHOD_READ:
        free(buffers[0].start);
        break;

    case IO_METHOD_MMAP:
        for (i = 0; i < n_buffers; ++i)
            if (-1 == munmap(buffers[i].start, buffers[i].length))
                errno_exit("munmap");
        break;

    case IO_METHOD_USERPTR:
        for (i = 0; i < n_buffers; ++i)
            free(buffers[i].start);
        break;
    }

    free(buffers);
}

void cv4l2::close_device(void)
{
    if (-1 == close(fd))
        errno_exit("close");

    fd = -1;
}

void cv4l2::set_format(int width, int height, const char *color)
{
    this->width = width;
    this->height = height;
    this->color = v4l2_fourcc(color[0], color[1], color[2], color[3]);

    stop_capturing();
    uninit_device();
    close_device();
    open_device();
    init_device();
}

void cv4l2::set_control(__u32 id, __s32 value)
{
    int value_old;
    struct v4l2_control ctrl;
    ctrl.id = id;
    if (-1 == ioctl(fd, VIDIOC_G_CTRL, &ctrl))
    {
        errno_exit("VIDIOC_G_CTRL");
    }
    value_old = ctrl.value;
    ctrl.value = value;
    if (-1 == ioctl(fd, VIDIOC_S_CTRL, &ctrl))
    {
        errno_exit("VIDIOC_S_CTRL");
    }
    if (-1 == ioctl(fd, VIDIOC_G_CTRL, &ctrl))
    {
        errno_exit("VIDIOC_G_CTRL");
    }
    printf("\nSet Control id: %04x, from %d to %d\n", ctrl.id, value_old, ctrl.value);
}

__s32 cv4l2::get_control(__u32 id)
{
    struct v4l2_control ctrl;
    ctrl.id = id;
    if (-1 == ioctl(fd, VIDIOC_G_CTRL, &ctrl))
    {
        errno_exit("VIDIOC_G_CTRL");
    }
    printf("\nGet Control id: %04x, value: %d\n", ctrl.id, ctrl.value);
    return ctrl.value;
}

/*********************************Extern*****************************************/

void *open(const char *dev_name)
{
    // void *handle;
    // handle = (void *)new cv4l2(dev_name);
    // ((cv4l2 *)handle)->open_device();
    // ((cv4l2 *)handle)->init_device();
    // return handle;
    return open(dev_name, 640, 400, "GREY");
}

void *open(const char *dev_name, int width, int height, const char *color)
{
    void *handle;
    handle = (void *)new cv4l2(dev_name, width, height, color);
    ((cv4l2 *)handle)->open_device();
    ((cv4l2 *)handle)->init_device();
    // ((cv4l2 *)handle)->set_control(V4L2_CID_EXPOSURE_AUTO,V4L2_EXPOSURE_AUTO);
    // ((cv4l2 *)handle)->set_control(V4L2_CID_EXPOSURE,800);
    // ((cv4l2 *)handle)->set_control(V4L2_CID_HBLANK, 816);//V4L2_CID_HBLANK: 816
    // ((cv4l2 *)handle)->set_control(V4L2_CID_VBLANK, 800); //V4L2_CID_VBLANK: 21
    // int value = ((cv4l2 *)handle)->get_control(V4L2_CID_BRIGHTNESS); //V4L2_CID_VBLANK: 21
    return handle;
}

void close(void *handle)
{
    ((cv4l2 *)handle)->uninit_device();
    ((cv4l2 *)handle)->close_device();
    delete ((cv4l2 *)handle);
}

void start(void *handle)
{
    ((cv4l2 *)handle)->start_capturing();
}

void stop(void *handle)
{
    ((cv4l2 *)handle)->stop_capturing();
}

struct buffer read(void *handle)
{
    return ((cv4l2 *)handle)->read_frame();
}

void setformat(void *handle, int width, int height, const char *color)
{
    ((cv4l2 *)handle)->set_format(width, height, color);
}

void setcontrol(void *handle, __u32 id, __s32 value)
{
    ((cv4l2 *)handle)->set_control(id, value);
}

__s32 getcontrol(void *handle, __u32 id)
{
    return ((cv4l2 *)handle)->get_control(id);
}

int main(int argc, char **argv)
{
    struct buffer frame;

    // void *handle = open("/dev/video0");
    void *handle = open("/dev/video0");

    // setformat(handle, 640, 400, "GREY");

    start(handle);

    for (int i = 0; i < 120 * 4; i++)
    {
        frame = read(handle);

        // printf("\ndata\n");
        // for (int i = 0; i < 640 * 4; i++)
        // {
        //     printf("%02x ", ((u_char*)frame.start)[i]);
        // }

        fflush(stderr);
        fprintf(stderr, ".");
        fflush(stdout);
    }

    stop(handle);
    close(handle);

    // cv4l2 *v4l2_0 = new cv4l2("/dev/video0");
    // v4l2_0->open_device();
    // v4l2_0->init_device();
    // v4l2_0->start_capturing();
    // v4l2_0->mainloop();
    // v4l2_0->stop_capturing();
    // v4l2_0->uninit_device();
    // v4l2_0->close_device();

    fprintf(stderr, "\n");

    return 0;
}
