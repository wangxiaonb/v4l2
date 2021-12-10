#include <stdio.h>
#include <stdlib.h>
#include <interface/mmal/mmal.h>
#include <interface/mmal/util/mmal_util.h>
#include <interface/mmal/util/mmal_connection.h>
#include <interface/mmal/util/mmal_util_params.h>

#define ENCODING MMAL_ENCODING_BGR24

class cmmal
{
private:
    MMAL_COMPONENT_T *render = NULL;
    MMAL_PORT_T *input;
    MMAL_POOL_T *pool;
    MMAL_BUFFER_HEADER_T *buffer;

    int width, height;

private:
    static void callback_vr_input(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
    {
        (void)port;
        mmal_buffer_header_release(buffer);
    }

public:
    cmmal()
    {
        mmal_component_create("vc.ril.video_render", &render);
        input = render->input[0];
    }

    ~cmmal()
    {
    }

    int open(int width, int height)
    {
        this->width = width;
        this->height = height;

        input->format->encoding = ENCODING;
        input->format->es->video.width = VCOS_ALIGN_UP(width, 32);
        input->format->es->video.height = VCOS_ALIGN_UP(height, 16);
        input->format->es->video.crop.x = 0;
        input->format->es->video.crop.y = 0;
        input->format->es->video.crop.width = width;
        input->format->es->video.crop.height = height;
        input->buffer_num = 3;

        mmal_port_format_commit(input);

        mmal_component_enable(render);

        mmal_port_parameter_set_boolean(input, MMAL_PARAMETER_ZERO_COPY, MMAL_TRUE);

        input->buffer_size = input->buffer_size_recommended;
        input->buffer_num = input->buffer_num_recommended;
        if (input->buffer_num < 2)
        {
            input->buffer_num = 2;
        }

        pool = mmal_port_pool_create(input, input->buffer_num, input->buffer_size);
        if (!pool)
        {
            printf("Oops, ,pool alloc failed\n");
            return -1;
        }

        mmal_port_enable(input, callback_vr_input);
        
        return 0;
    }

    void close()
    {
        if (input != NULL)
        {
            mmal_port_disable(input);
            input = NULL;
        }
        if (render != NULL)
        {
            mmal_component_destroy(render);
            render = NULL;
        }
    }

    void set(int32_t layer, uint32_t alpha,
                  int32_t x, int32_t y, int32_t width, int32_t height)
    {
        MMAL_DISPLAYREGION_T param;
        param.hdr.id = MMAL_PARAMETER_DISPLAYREGION;
        param.hdr.size = sizeof(MMAL_DISPLAYREGION_T);

        param.set = MMAL_DISPLAY_SET_LAYER;
        param.layer = layer; //On top of most things

        param.set |= MMAL_DISPLAY_SET_ALPHA;
        param.alpha = alpha; //0 = transparent, 255 = opaque

        param.set |= (MMAL_DISPLAY_SET_DEST_RECT | MMAL_DISPLAY_SET_FULLSCREEN);
        param.fullscreen = 0;
        param.dest_rect.x = x;
        param.dest_rect.y = y;
        param.dest_rect.width = width;
        param.dest_rect.height = height;
        mmal_port_parameter_set(input, &param.hdr);
    }

    void show(uint8_t *data)
    {
        buffer = mmal_queue_wait(pool->queue);

        // Write something into the buffer.
        // memset(buffer->data, (i << 4) & 0xff, buffer->alloc_size);
        // memset(buffer->data, i, buffer->alloc_size / 2);
        // memset(buffer->data + buffer->alloc_size / 2, 0xff - i, buffer->alloc_size / 2);
        memcpy(buffer->data, data, buffer->alloc_size);

        buffer->length = buffer->alloc_size;
        mmal_port_send_buffer(input, buffer);
    }
};