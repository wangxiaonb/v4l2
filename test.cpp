#include "v4l2.hpp"

int main(int argc, char **argv)
{
    struct buffer frame;

    void *handle = open("/dev/video0");
    // unsigned long result = (unsigned long)handle;
    // printf("c++____handle: open %lu\n",result);

    start(handle);

    for (int i = 0; i < 100; i++)
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

    fprintf(stderr, "\n");

    return 0;
}