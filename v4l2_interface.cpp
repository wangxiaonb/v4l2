// #include "v4l2.h"

// extern "C" int add(int a, int b)
// {
//     return a + b;
// }

// cv4l2* open_camera(const char *dev_name)
// {
//     return new cv4l2(dev_name);
// }

// int main(int argc, char **argv)
// {
//     cv4l2 *v4l2_0 = new cv4l2("/dev/video0");

//     v4l2_0->open_device();
//     v4l2_0->init_device();
//     v4l2_0->start_capturing();
//     v4l2_0->mainloop();
//     v4l2_0->stop_capturing();
//     v4l2_0->uninit_device();
//     v4l2_0->close_device();

//     fprintf(stderr, "\n");

//     return 0;
// }