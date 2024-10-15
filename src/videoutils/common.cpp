#include "common.h"

Frame::Frame()
{
    pkt = av_packet_alloc();
}

Frame::~Frame()
{
    av_packet_free(&pkt);
}

template <typename T>
T adj_value(T min, T max, int index, int frame_ratio_scale)
{
    return min + (max - min) * index / frame_ratio_scale;
}

void fix_timestamp(CycleBuffer<Frame, 4> &buffer, int frame_ratio_scale)
{
    for (size_t i = 1; i < frame_ratio_scale; i++)
    {
        buffer[i].frame_timestamp.pts = adj_value(buffer.head().frame_timestamp.pts, 
                                                    buffer.tail().frame_timestamp.pts, 
                                                    i, frame_ratio_scale);
        buffer[i].frame_timestamp.pkt_dts = adj_value(buffer.head().frame_timestamp.pkt_dts,
                                                    buffer.tail().frame_timestamp.pkt_dts,
                                                    i, frame_ratio_scale);
        buffer[i].frame_timestamp.duration = buffer.head().frame_timestamp.duration / frame_ratio_scale;
        buffer[i].pkt_timestamp.pts = adj_value(buffer.head().pkt_timestamp.pts,
                                                    buffer.tail().pkt_timestamp.pts,
                                                    i, frame_ratio_scale);
        buffer[i].pkt_timestamp.dts = adj_value(buffer.head().pkt_timestamp.dts,
                                                buffer.tail().pkt_timestamp.dts,
                                                i, frame_ratio_scale);
        buffer[i].pkt_timestamp.duration = buffer.head().pkt_timestamp.duration / frame_ratio_scale;
        buffer[i].stream_index = buffer.head().stream_index;
    }
    buffer.head().frame_timestamp.duration = buffer.head().frame_timestamp.duration / frame_ratio_scale;
    buffer.head().pkt_timestamp.duration = buffer.head().pkt_timestamp.duration / frame_ratio_scale;
}

void fix_duration(Frame &frame, int frame_ratio_scale)
{
    frame.frame_timestamp.duration /= frame_ratio_scale;
    frame.pkt_timestamp.duration /= frame_ratio_scale;
}
