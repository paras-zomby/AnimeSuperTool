#ifndef VIDEOREADER_H
#define VIDEOREADER_H

#include <string>
#include <sstream>

#include "net.h"
#include "common.h"

class VideoReader
{
public:
    VideoReader();
    VideoReader(const std::string &filename);
    VideoReader(VideoReader &&) = delete;
    VideoReader(const VideoReader &) = delete;
    VideoReader &operator=(VideoReader &&) = delete;
    VideoReader &operator=(const VideoReader &) = delete;
    ~VideoReader();

    bool open_video(const std::string &filename);
    bool read_frame(Frame& frame);
    void close_video();

    inline std::string get_error_string() const { return error_string.str(); }
    inline const AVStream *const *const get_streams() const { return ifmt_ctx->streams; }
    inline const AVStream *const get_video_stream() const { return ifmt_ctx->streams[video_stream_index]; }
    inline int get_stream_nb() const { return ifmt_ctx->nb_streams; }
    inline bool is_opened() const { return is_opened_; }

private:
    AVFormatContext *ifmt_ctx = nullptr;
    AVCodecContext *codec_ctx = nullptr;
    const AVCodec *codec = nullptr;
    AVFrame *inframe = nullptr;
    struct SwsContext *sws_ctx = nullptr;
    int video_stream_index = -1;

    std::stringstream error_string;
    bool is_opened_ = false;
};



#endif // VIDEOREADER_H
