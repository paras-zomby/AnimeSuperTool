#ifndef CONFIG_H
#define CONFIG_H

extern "C"
{
#include <libavformat/avformat.h>
}

struct Params
{
    int zoom_scale = 1;
    int frame_ratio_scale = 1;
    bool denoise = false;
    int num_threads_pergpu = 1;
    int tile_num = 2;
    int gpu = 0;
    int ifrnet_type = 2;
    float imagezoom_bitrate_scale = 0.7;
    float framerate_bitrate_scale = 0.5;
    AVPixelFormat frame_fmt = AV_PIX_FMT_YUV420P;
};

#endif // !CONFIG_H