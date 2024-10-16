#include "videowriter.h"

VideoWriter::VideoWriter()
{
}

VideoWriter::VideoWriter(const std::string &filename)
{
    open_video_file(filename);
}

VideoWriter::~VideoWriter()
{
    close_video();
}

void VideoWriter::close_video()
{
    write_video_trailer();
    deinit_video();
    reset_params();
    close_video_file();
}

bool VideoWriter::open_video_file(const std::string &filename)
{
    // 打开输出文件
    avformat_alloc_output_context2(&ofmt_ctx, nullptr, nullptr, filename.c_str());
    if (!ofmt_ctx)
    {
        error_string << "Could not create output context" << std::endl;
        close_video_file();
        return false;
    }

    // 查找编码器
    for (size_t i = 0; i < encoderlist.size(); ++i)
    {
        codec = avcodec_find_encoder_by_name(encoderlist[i]);
        if (codec)
            break;
    }
    
    if (!codec)
    {
        error_string << "Codec not found" << std::endl;
        close_video_file();
        return false;
    }

    // 初始化编码器上下文
    codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx)
    {
        error_string << "Could not allocate video codec context" << std::endl;
        close_video_file();
        return false;
    }
    output_filename = filename;
    is_open_video_file = true;
    return true;
}

bool VideoWriter::write_video_trailer()
{
    if (!is_video_trailer)
    {
        // 写文件尾
        if (av_write_trailer(ofmt_ctx) < 0)
        {
            error_string << "Failed to write video file trailer" << std::endl;
            return false;
        }
    }
    is_video_trailer = true;
    return true;
}

void VideoWriter::close_video_file()
{
    if (codec_ctx)
    {
        avcodec_free_context(&codec_ctx);
        codec_ctx = nullptr;
    }
    if (ofmt_ctx)
    {
        avformat_free_context(ofmt_ctx);
        ofmt_ctx = nullptr;
    }
    is_open_video_file = false;
}

bool VideoWriter::set_params(const VideoReader &reader, const Params& params)
{
    if (!is_open_video_file)
    {
        error_string << "Video file is not opened" << std::endl;
        return false;
    }
    if (!reader.is_opened())
    {
        error_string << "Video reader is not opened" << std::endl;
        return false;
    }
    // 设置编码器参数
    AVCodecParameters *codecpar = reader.get_video_stream()->codecpar;
    ori_time_base = reader.get_video_stream()->time_base;
    AVRational time_base = ori_time_base;
    AVDictionary *opts = nullptr;
    time_base.den *= params.frame_ratio_scale;
    codec_ctx->time_base = time_base;
    codec_ctx->pix_fmt = params.frame_fmt;
    codec_ctx->width = codecpar->width * params.zoom_scale;
    codec_ctx->height = codecpar->height * params.zoom_scale;
    codec_ctx->bit_rate = codecpar->bit_rate * (
        (params.zoom_scale - 1) * params.imagezoom_bitrate_scale + \
        (params.frame_ratio_scale - 1) * params.framerate_bitrate_scale + 1
    );
#if LIBAVUTIL_VERSION_MAJOR > 58  // use different var for low version
    AVRational framerate = codecpar->framerate;
    framerate.num *= params.frame_ratio_scale;
    codec_ctx->framerate = framerate;
#endif

    if (strcmp(codec->name, "hevc_amf") == 0)
    {
    if (av_dict_set(&opts, "rc", "vbr_peak", 0) < 0)
    {
        error_string << "Failed to set codec options" << std::endl;
        av_dict_free(&opts);
        reset_params();
        return false;
    }
    if (av_dict_set_int(&opts, "maxrate", 5 * codec_ctx->bit_rate, 0) < 0)
    {
        error_string << "Failed to set codec options" << std::endl;
        av_dict_free(&opts);
        reset_params();
        return false;
    }
    if (av_dict_set(&opts, "quality", "quality", 0) < 0)
    {
        error_string << "Failed to set codec options" << std::endl;
        av_dict_free(&opts);
        reset_params();
        return false;
    }
    }
    else if (strcmp(codec->name, "hevc_nvenc") == 0)
    {
        if (av_dict_set(&opts, "rc", "vbr", 0) < 0)
        {
            error_string << "Failed to set codec options" << std::endl;
            av_dict_free(&opts);
            reset_params();
            return false;
        }
        if (av_dict_set_int(&opts, "maxrate", 5 * codec_ctx->bit_rate, 0) < 0)
        {
            error_string << "Failed to set codec options" << std::endl;
            av_dict_free(&opts);
            reset_params();
            return false;
        }
        if (av_dict_set_int(&opts, "bitrate", codec_ctx->bit_rate, 0) < 0)
        {
            error_string << "Failed to set codec options" << std::endl;
            av_dict_free(&opts);
            reset_params();
            return false;
        }
    }
    else 
    {
        fprintf(stderr, "Warning: unknown encoder: %s, using default config.\n", codec->name);
    }

    if (avcodec_open2(codec_ctx, codec, &opts) < 0)
    {
        error_string << "Failed to open codec" << std::endl;
        reset_params();
        return false;
    }
    av_dict_free(&opts);

    new_streams = (AVStream **)av_malloc_array(reader.get_stream_nb(), sizeof(AVStream *));
    streams_mapping = (int *)av_malloc_array(reader.get_stream_nb(), sizeof(int));
    for (size_t i = 0; i < reader.get_stream_nb(); i++)
    {
        AVStream *stream = avformat_new_stream(ofmt_ctx, nullptr);
        if (!stream)
        {
            error_string << "Failed to allocate stream" << std::endl;
            reset_params();
            return false;
        }
        if (i == reader.get_video_stream()->index)
        {
            video_stream_index = stream->index;
            if (avcodec_parameters_from_context(stream->codecpar, codec_ctx) < 0)
            {
                error_string << "Failed to copy codec parameters to output video stream" << std::endl;
                reset_params();
                return false;
            }
            stream->time_base = codec_ctx->time_base;
        }
        else
        {
            if (avcodec_parameters_copy(stream->codecpar, reader.get_streams()[i]->codecpar) < 0)
            {
                error_string << "Failed to copy codec parameters to output other stream" << std::endl;
                reset_params();
                return false;
            }
            stream->time_base = reader.get_streams()[i]->time_base;
        }
        new_streams[i] = stream;
        streams_mapping[i] = stream->index;
    }

    av_dump_format(ofmt_ctx, 0, output_filename.c_str(), 1);

    is_set_params = true;
    return true;
}

void VideoWriter::reset_params()
{
    if (new_streams)
    {
        av_free(new_streams);
        new_streams = nullptr;
    }
    if (streams_mapping)
    {
        av_free(streams_mapping);
        streams_mapping = nullptr;
    }
    video_stream_index = -1;
    AVCodecParameters *tmp = avcodec_parameters_alloc();
    if (tmp)
    {
        // return the codec parameters to the default state
        avcodec_parameters_to_context(codec_ctx, tmp);
        avcodec_parameters_free(&tmp);
    }
    is_set_params = false;
}

bool VideoWriter::init_video()
{
    if (!is_open_video_file)
    {
        error_string << "Video file is not opened" << std::endl;
        return false;
    }
    if (!is_set_params)
    {
        error_string << "Video params is not set" << std::endl;
        return false;
    }
    // 打开输出文件
    if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
    {
        if (avio_open(&ofmt_ctx->pb, output_filename.c_str(), AVIO_FLAG_WRITE) < 0)
        {
            error_string << "Could not open output file" << output_filename << std::endl;
            deinit_video();
            return false;
        }
    }
    // 写文件头
    if (avformat_write_header(ofmt_ctx, nullptr) < 0)
    {
        error_string << "Failed to write video file header" << std::endl;
        deinit_video();
        return false;
    }

    sws_ctx = sws_getContext(codec_ctx->width, codec_ctx->height,
                            AV_PIX_FMT_RGB24,
                            codec_ctx->width, codec_ctx->height,
                            codec_ctx->pix_fmt,
                            SWS_BILINEAR, NULL, NULL, NULL);

    if (!sws_ctx)
    {
        error_string << "Could not initialize the conversion context" << std::endl;
        deinit_video();
        return false;
    }
    outframe = av_frame_alloc();
    if (!outframe)
    {
        error_string << "Could not allocate video frame" << std::endl;
        deinit_video();
        return false;
    }
    outframe->format = codec_ctx->pix_fmt;
    outframe->width = codec_ctx->width;
    outframe->height = codec_ctx->height;
    if (av_frame_get_buffer(outframe, 0) < 0)
    {
        fprintf(stderr, "Could not allocate the video frame data\n");
        return -1;
    }
    outpkt = av_packet_alloc();
    if (!outpkt)
    {
        error_string << "Could not allocate the video packet" << std::endl;
        deinit_video();
        return false;
    }
    is_init_video = true;
    is_video_trailer = false;
    return true;
}

void VideoWriter::deinit_video()
{
    if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
    {
        avio_closep(&ofmt_ctx->pb);
    }
    if (sws_ctx)
    {
        sws_freeContext(sws_ctx);
        sws_ctx = nullptr;
    }
    if (outframe)
    {
        av_frame_free(&outframe);
        outframe = nullptr;
    }
    if (outpkt)
    {
        av_packet_free(&outpkt);
        outpkt = nullptr;
    }
    is_init_video = false;
}

bool VideoWriter::write_frame(const Frame &frame)
{
    if (frame.type == FrameType::FIN)
    {
        return true;
    }
    else if (frame.type == FrameType::VIDEO)
    {
        // // 将图片保存为帧
        uint8_t *outdata[1] = {(uint8_t *)frame.img.data};
        int line_size = frame.img.w * 3;
        sws_scale(sws_ctx, outdata, &line_size, 0, frame.img.h, outframe->data, outframe->linesize);

        outframe->pts = av_rescale_q(frame.frame_timestamp.pts, ori_time_base, codec_ctx->time_base);
        outframe->pkt_dts = av_rescale_q(frame.frame_timestamp.pkt_dts, ori_time_base, codec_ctx->time_base);
#if LIBAVUTIL_VERSION_MAJOR < 58  // use different var for low version
        outframe->pkt_duration = av_rescale_q(frame.frame_timestamp.duration, ori_time_base, codec_ctx->time_base);        
#else
        outframe->duration = av_rescale_q(frame.frame_timestamp.duration, ori_time_base, codec_ctx->time_base);        
#endif
        // 编码帧
        int retno = avcodec_send_frame(codec_ctx, outframe);
        if (retno < 0)
        {
            error_string << "Error sending a frame for encoding, errorno: " << retno << std::endl;
            return false;
        }
        while (avcodec_receive_packet(codec_ctx, outpkt) == 0)
        {
            outpkt->pts = frame.pkt_timestamp.pts;
            outpkt->dts = frame.pkt_timestamp.dts;
            outpkt->duration = frame.pkt_timestamp.duration;

            outpkt->stream_index = streams_mapping[frame.stream_index];
            outpkt->pos = -1;
            av_packet_rescale_ts(outpkt, ori_time_base, codec_ctx->time_base);
            if (av_interleaved_write_frame(ofmt_ctx, outpkt) < 0)
            {
                error_string << "Error while writing output packet" << std::endl;
                return false;
            }
        }
    }
    else if (frame.type == FrameType::OTHER)
    {
        frame.pkt->stream_index = streams_mapping[frame.pkt->stream_index];
        if (av_interleaved_write_frame(ofmt_ctx, frame.pkt) < 0)
        {
            error_string << "Error while writing output packet" << std::endl;
            return false;
        }
    }
    return true;
}

