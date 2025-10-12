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

/**
 * @brief A list of preferred hardware encoders.
 */
constexpr std::array<char[15], 4> encoderlist = {"hevc_amf", "hevc_nvenc", "hevc_qsv", "libx265"};

/**
 * @enum FrameType
 * @brief An enumeration of frame types used in video processing.
 */
enum class FrameType : int
{
    VIDEO,    ///< Represents a video frame.
    OTHER,    ///< Represents a non-video frame (e.g., audio, subtitle).
    FIN = -1, ///< Represents the end of the stream.
};

/**
 * @struct Frame
 * @brief A structure to hold a single frame of data, which can be video or other data.
 */
struct Frame
{
    ncnn::Mat img;  ///< The image data for a video frame, stored as an ncnn::Mat.
    FrameType type; ///< The type of the frame (e.g., VIDEO, OTHER, FIN).

    // Audio and other type frame packet
    AVPacket *pkt; ///< The raw packet data for non-video frames.
    // Video frame timestamp
    /**
     * @struct frame_timestamp
     * @brief Timestamp information for a decoded video frame.
     */
    struct
    {
        int64_t pts;      ///< Presentation timestamp.
        int64_t pkt_dts;  ///< Decoding timestamp of the packet that contained this frame.
        int64_t duration; ///< The duration of the frame.
    } frame_timestamp;
    /**
     * @struct pkt_timestamp
     * @brief Timestamp information for a packet.
     */
    struct
    {
        int64_t pts;      ///< Presentation timestamp.
        int64_t dts;      ///< Decoding timestamp.
        int64_t duration; ///< The duration of the packet.
    } pkt_timestamp;

    int stream_index; ///< The index of the stream this frame belongs to.

    /**
     * @brief Default constructor for Frame.
     */
    Frame();
    Frame(const Frame &) = delete;
    Frame(Frame &&) = delete;
    Frame &operator=(const Frame &) = delete;
    Frame &operator=(Frame &&) = delete;
    /**
     * @brief Destructor for Frame, which deallocates the AVPacket.
     */
    ~Frame();
};

/**
 * @brief Adjusts the timestamps of frames in a buffer after interpolation.
 * @param buffer A cycle buffer containing frames whose timestamps need to be fixed.
 * @param frame_ratio_scale The scaling factor for the frame rate (e.g., 2 for doubling the frame rate).
 */
void fix_timestamp(CycleBuffer<Frame, 4> &buffer, int frame_ratio_scale);

/**
 * @brief Adjusts the duration of a single frame.
 * @param frame The frame whose duration needs to be fixed.
 * @param frame_ratio_scale The scaling factor for the frame rate.
 */
void fix_duration(Frame &frame, int frame_ratio_scale);

#endif // COMMON_H