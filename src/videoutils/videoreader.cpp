#include "videoreader.h"

VideoReader::VideoReader()
{
}

VideoReader::VideoReader(const std::string &filename)
{
    open_video(filename);
}

VideoReader::~VideoReader()
{
    close_video();
}

bool VideoReader::open_video(const std::string &filename)
{
    // 打开输入文件
    if (avformat_open_input(&ifmt_ctx, filename.c_str(), NULL, NULL) < 0)
    {
        error_string << "Unable to open file: " << filename << std::endl;
        return false;
    }

    // 读取流信息
    if (avformat_find_stream_info(ifmt_ctx, NULL) < 0)
    {
        error_string << "Unable to read stream info: " << filename << std::endl;
        close_video();
        return false;
    }

    // 打印格式信息
    av_dump_format(ifmt_ctx, 0, filename.c_str(), 0);

    // 查找视频流与解码器
    video_stream_index = av_find_best_stream(
        ifmt_ctx,
        AVMEDIA_TYPE_VIDEO,
        -1, -1, &codec, 0);
    if (video_stream_index < 0)
    {
        error_string << "Failed to find a Video steam: " << filename << std::endl;
        close_video();
        return false;
    }
    if (!codec)
    {
        error_string << "Failed to find a decoder: " << filename << std::endl;
        close_video();
        return false;
    }

    // 初始化解码器上下文
    codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx)
    {
        error_string << "Failed to allocate codec context" << std::endl;
        close_video();
        return false;
    }
    AVCodecParameters *codecpar = ifmt_ctx->streams[video_stream_index]->codecpar;
    // 将解码器参数拷贝到解码器上下文
    if (avcodec_parameters_to_context(codec_ctx, codecpar) < 0)
    {
        error_string << "Failed to copy codec parameters to decoder context" << std::endl;
        close_video();
        return false;
    }

    if (avcodec_open2(codec_ctx, codec, nullptr) < 0)
    {
        error_string << "Failed to open decodec" << std::endl;
        close_video();
        return false;
    }

    sws_ctx = sws_getContext(codec_ctx->width, codec_ctx->height,
                             codec_ctx->pix_fmt,
                             codec_ctx->width, codec_ctx->height,
                             AV_PIX_FMT_RGB24,
                             SWS_BILINEAR, NULL, NULL, NULL);

    if (!sws_ctx)
    {
        error_string << "Failed to initialize the conversion context" << std::endl;
        close_video();
        return false;
    }

    inframe = av_frame_alloc();
    if (!inframe)
    {
        error_string << "Could not allocate video frame" << std::endl;
        close_video();
        return false;
    }
    is_opened_ = true;
    return true;
}

bool VideoReader::read_frame(Frame &frame)
{
    int ret = avcodec_receive_frame(codec_ctx, inframe);
    while (ret == AVERROR(EAGAIN))
    {
        if (frame.pkt)
        {
            av_packet_unref(frame.pkt);
        }
        if (av_read_frame(ifmt_ctx, frame.pkt) < 0)
        {
            error_string << "Failed to read a frame, maybe file meets EOF." << std::endl;
            return false;
        }
        if (frame.pkt->stream_index != video_stream_index)
        {
            frame.type = FrameType::OTHER;
            return true;
        }
        if (avcodec_send_packet(codec_ctx, frame.pkt) != 0)
        {
            error_string << "Failed to send packet to decoder" << std::endl;
            return false;
        }
        ret = avcodec_receive_frame(codec_ctx, inframe);
    }
    if (ret < 0)
    {
        error_string << "Failed to receive frame from decoder" << std::endl;
        return false;
    }
    frame.type = FrameType::VIDEO;

    frame.frame_timestamp.pts = inframe->pts;
    frame.frame_timestamp.pkt_dts = inframe->pkt_dts;
    frame.frame_timestamp.duration = inframe->duration;
    frame.stream_index = video_stream_index;

    frame.pkt_timestamp.pts = frame.pkt->pts;
    frame.pkt_timestamp.dts = frame.pkt->dts;
    frame.pkt_timestamp.duration = frame.pkt->duration;

    frame.img.create(inframe->width, inframe->height, (size_t)3, 3);
    uint8_t *indata[1] = {(uint8_t *)frame.img.data};
    int inlinesize = frame.img.w * 3;
    sws_scale(sws_ctx, inframe->data, inframe->linesize, 0,
              inframe->height, indata, &inlinesize);
    return true;
}

void VideoReader::close_video()
{
    if (!inframe)
        av_frame_free(&inframe);
    if (!sws_ctx)
        sws_freeContext(sws_ctx);
    if (!codec_ctx)
        avcodec_free_context(&codec_ctx);
    if (!ifmt_ctx)
        avformat_close_input(&ifmt_ctx);
    video_stream_index = -1;
    is_opened_ = false;
}

