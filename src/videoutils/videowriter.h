#ifndef VIDEO_WRITE_H
#define VIDEO_WRITE_H

#include "common.h"
#include "utils/config.h"
#include "videoreader.h"

#include <string>
#include <sstream>

/**
 * @class VideoWriter
 * @brief A class for writing video files using FFmpeg.
 */
class VideoWriter
{
public:
    /**
     * @brief Default constructor for VideoWriter.
     */
    VideoWriter();
    /**
     * @brief Construct a new VideoWriter object and open a video file for writing.
     * @param filename The path to the output video file.
     */
    VideoWriter(const std::string &filename);
    /**
     * @brief Destroy the VideoWriter object and release resources.
     */
    ~VideoWriter();

    /**
     * @brief Open a video file for writing.
     * @param filename The path to the output video file.
     * @return true if the file was opened successfully, false otherwise.
     */
    bool open_video_file(const std::string &filename);
    /**
     * @brief Set the parameters for the output video.
     * @param reader The VideoReader object for the input video, used to copy stream parameters.
     * @param params The custom parameters for the output video.
     * @return true if parameters were set successfully, false otherwise.
     */
    bool set_params(const VideoReader &reader, const Params &params = {});
    /**
     * @brief Initialize the video writer.
     * @return true if initialization was successful, false otherwise.
     */
    bool init_video();
    /**
     * @brief Write a single frame to the video file.
     * @param frame The frame to write.
     * @return true if the frame was written successfully, false otherwise.
     */
    bool write_frame(const Frame &frame);
    /**
     * @brief Close the video file and finalize writing.
     */
    void close_video();

    /**
     * @brief Get the error string from the last operation.
     * @return A string containing any error messages.
     */
    inline std::string get_error_string() const { return error_string.str(); }
    /**
     * @brief Check if the video writer is ready to write frames.
     * @return true if the writer is opened, parameters are set, and it's initialized.
     */
    inline bool is_opened() const { return is_open_video_file && is_set_params && is_init_video; }

private:
    bool write_video_trailer();
    void close_video_file();
    void reset_params();
    void deinit_video();
    int copy_chapters(const AVChapter *const *const src, AVChapter ***pdst,
                      int nb_chapters, AVRational dsttimebase);

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

