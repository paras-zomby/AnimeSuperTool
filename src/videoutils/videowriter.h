#ifndef VIDEO_WRITE_H
#define VIDEO_WRITE_H

#include "common.h"
#include "utils/config.h"
#include "VideoReader.h"

#include <string>
#include <sstream>

class VideoWriter
{
public:
    VideoWriter();
    VideoWriter(const std::string &filename);
    ~VideoWriter();

    bool open_video_file(const std::string &filename);
    bool set_params(const VideoReader &reader, const Params &params = {});
    bool init_video();
    bool write_frame(const Frame &frame);
    void close_video();

    inline std::string get_error_string() const { return error_string.str(); }
    inline bool is_opened() const { return is_open_video_file && is_set_params && is_init_video; }

private:
    bool write_video_trailer();
    void close_video_file();
    void reset_params();
    void deinit_video();

private:
    std::string output_filename;
    AVFormatContext *ofmt_ctx = nullptr;
    const AVCodec *codec = nullptr;
    AVCodecContext *codec_ctx = nullptr;
    AVStream **new_streams = nullptr;
    AVFrame *outframe = nullptr;
    AVPacket *outpkt = nullptr;
    struct SwsContext *sws_ctx = nullptr;
    int *streams_mapping = nullptr;
    int video_stream_index = -1;
    AVRational ori_time_base;

    std::stringstream error_string;
    bool is_video_trailer = true;
    bool is_open_video_file = false;
    bool is_init_video = false;
    bool is_set_params = false;
};



#endif // VIDEO_WRITE_H

