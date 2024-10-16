#ifndef COMMON_H
#define COMMON_H

#include <array>
#include "net.h"
#include "utils/cyclebuffer.hpp"
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

constexpr std::array<char[15], 4> encoderlist = {"hevc_amf", "hevc_nvenc", "hevc_qsv", "libx265"};

enum class FrameType : int
{
    VIDEO,
    OTHER,
    FIN = -1,
};

struct Frame
{
    ncnn::Mat img;
    FrameType type;

    // Audio and other type frame packet
    AVPacket *pkt;
    // Video frame timestamp
    struct 
    {
        int64_t pts;
        int64_t pkt_dts;
        int64_t duration;
    } frame_timestamp;
    struct 
    {
        int64_t pts;
        int64_t dts;
        int64_t duration;
    } pkt_timestamp;

    int stream_index;

    Frame();
    Frame(const Frame &) = delete;
    Frame(Frame &&) = delete;
    Frame &operator=(const Frame &) = delete;
    Frame &operator=(Frame &&) = delete;
    ~Frame();
};

void fix_timestamp(CycleBuffer<Frame, 4> &buffer, int frame_ratio_scale);
void fix_duration(Frame &frame, int frame_ratio_scale);

#endif // COMMON_H