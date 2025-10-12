#ifndef VIDEOREADER_H
#define VIDEOREADER_H

#include <string>
#include <sstream>

#include "net.h"
#include "common.h"

/**
 * @class VideoReader
 * @brief A class for reading video files using FFmpeg.
 */
class VideoReader
{
public:
    /**
     * @brief Default constructor for VideoReader.
     */
    VideoReader();
    /**
     * @brief Construct a new VideoReader object and open a video file.
     * @param filename The path to the video file.
     */
    VideoReader(const std::string &filename);
    VideoReader(VideoReader &&) = delete;
    VideoReader(const VideoReader &) = delete;
    VideoReader &operator=(VideoReader &&) = delete;
    VideoReader &operator=(const VideoReader &) = delete;
    /**
     * @brief Destroy the VideoReader object and release resources.
     */
    ~VideoReader();

    /**
     * @brief Open a video file.
     * @param filename The path to the video file.
     * @return true if the video was opened successfully, false otherwise.
     */
    bool open_video(const std::string &filename);
    /**
     * @brief Read a single frame from the video.
     * @param frame The output frame.
     * @return true if a frame was read successfully, false otherwise (e.g., end of file).
     */
    bool read_frame(Frame& frame);
    /**
     * @brief Close the video file and release resources.
     */
    void close_video();

    /**
     * @brief Get the error string from the last operation.
     * @return A string containing any error messages.
     */
    inline std::string get_error_string() const { return error_string.str(); }
    /**
     * @brief Get the streams from the video file.
     * @return A pointer to the array of AVStream pointers.
     */
    inline const AVStream *const *const get_streams() const { return ifmt_ctx->streams; }
    /**
     * @brief Get the video stream from the file.
     * @return A pointer to the video AVStream.
     */
    inline const AVStream *const get_video_stream() const { return ifmt_ctx->streams[video_stream_index]; }
    /**
     * @brief Get the chapters from the video file.
     * @return A pointer to the array of AVChapter pointers.
     */
    inline const AVChapter *const*const get_chapters() const { return ifmt_ctx->chapters; }
    /**
     * @brief Get the number of streams in the video file.
     * @return The number of streams.
     */
    inline int get_stream_nb() const { return ifmt_ctx->nb_streams; }
    /**
     * @brief Get the number of chapters in the video file.
     * @return The number of chapters.
     */
    inline int get_chapter_nb() const { return ifmt_ctx->nb_chapters; }
    /**
     * @brief Check if a video file is currently open.
     * @return true if a video is open, false otherwise.
     */
    inline bool is_opened() const { return is_opened_; }

private:
    AVFormatContext *ifmt_ctx = nullptr;
    AVCodecContext *codec_ctx = nullptr;
    AVFrame *inframe = nullptr;
#if LIBAVUTIL_VERSION_MAJOR < 58  // use different var for low version
    AVCodec *codec = nullptr;
#else
    const AVCodec *codec = nullptr;
#endif
    struct SwsContext *sws_ctx = nullptr;
    int video_stream_index = -1;

    std::stringstream error_string;
    bool is_opened_ = false;
};



#endif // VIDEOREADER_H
