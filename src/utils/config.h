#ifndef CONFIG_H
#define CONFIG_H

extern "C"
{
#include <libavformat/avformat.h>
}

/**
 * @struct Params
 * @brief A structure to hold configuration parameters for video processing.
 */
struct Params
{
    /**
     * @brief The scaling factor for image resolution enhancement.
     * A value of 1 means no scaling.
     */
    int zoom_scale = 1;
    /**
     * @brief The scaling factor for frame rate interpolation.
     * A value of 1 means no interpolation.
     */
    int frame_ratio_scale = 1;
    /**
     * @brief A flag to enable or disable denoising.
     */
    bool denoise = false;
    /**
     * @brief The number of processing threads to use per GPU.
     */
    int num_threads_pergpu = 1;
    /**
     * @brief The number of tiles to split the image into for processing.
     * This is used to manage memory usage.
     */
    int tile_num = 2;
    /**
     * @brief The ID of the GPU to use for processing.
     */
    int gpu = 0;
    /**
     * @brief The type of IFRNet model to use.
     */
    int ifrnet_type = 2;
    /**
     * @brief The scaling factor for the bitrate when image resolution is increased.
     */
    float imagezoom_bitrate_scale = 0.7;
    /**
     * @brief The scaling factor for the bitrate when the frame rate is increased.
     */
    float framerate_bitrate_scale = 0.5;
    /**
     * @brief The pixel format for the output video frames.
     */
    AVPixelFormat frame_fmt = AV_PIX_FMT_YUV420P;
};

#endif // !CONFIG_H