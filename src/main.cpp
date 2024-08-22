#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
}
#include "net.h"
#include "utils/progressbar.hpp"

int main() {
    const char *filename = "../video.mp4"; // 视频文件路径
    const char *output_filename = "../output.mp4";  // 输出文件路径
    AVFormatContext *ifmt_ctx = nullptr;
    AVFormatContext *ofmt_ctx = nullptr;
    AVCodecContext *decoder_codec_ctx = nullptr;
    const AVCodec *decoder_codec = nullptr;
    AVStream **new_streams = nullptr;
    int *streams_mapping = nullptr;
    AVFrame *input_frame = nullptr;
    AVFrame *output_frame = nullptr;
    AVPacket* inpkt;
    AVPacket* outpkt;    

    // 打开输入文件
    if (avformat_open_input(&ifmt_ctx, filename, NULL, NULL) < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "无法打开文件 %s\n", filename);
        return -1;
    }
    // 打开输出文件
    avformat_alloc_output_context2(&ofmt_ctx, nullptr, nullptr, output_filename);
    if (!ofmt_ctx)
    {
        fprintf(stderr, "Could not create output context\n");
        return -1;
    }

    // 读取流信息
    if (avformat_find_stream_info(ifmt_ctx, NULL) < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "无法获取流信息\n");
        avformat_close_input(&ifmt_ctx);
        return -1;
    }
    // 打印格式信息
    av_dump_format(ifmt_ctx, 0, filename, 0);

    // 查找视频流与解码器
    int input_video_stream_index = av_find_best_stream(
        ifmt_ctx,
        AVMEDIA_TYPE_VIDEO,
        -1, -1, &decoder_codec, 0);
    if (input_video_stream_index < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "没有找到视频流\n");
        avformat_close_input(&ifmt_ctx);
        return -1;
    }
    if (!decoder_codec)
    {
        av_log(NULL, AV_LOG_ERROR, "找不到解码器\n");
        avformat_close_input(&ifmt_ctx);
        return -1;
    }

    // 查找其他流
    new_streams = (AVStream **)av_malloc_array(ifmt_ctx->nb_streams, sizeof(AVStream *));
    streams_mapping = (int *)av_malloc_array(ifmt_ctx->nb_streams, sizeof(int));
    for (size_t i = 0; i < ifmt_ctx->nb_streams; i++)
    {
        if (i == input_video_stream_index)
            continue;
        AVStream *stream = avformat_new_stream(ofmt_ctx, nullptr);
        avcodec_parameters_copy(stream->codecpar, ifmt_ctx->streams[i]->codecpar);
        stream->time_base = ifmt_ctx->streams[i]->time_base;
        if (!stream)
        {
            fprintf(stderr, "Failed to allocate stream\n");
            return -1;
        }
        new_streams[i] = stream;
        streams_mapping[i] = stream->index;
    }

    // 查找编码器
    // const AVCodec *encoder_codec = avcodec_find_encoder(AV_CODEC_ID_H265);
    const AVCodec *encoder_codec = avcodec_find_encoder_by_name("hevc_amf");
    if (!encoder_codec)
    {
        fprintf(stderr, "Codec not found\n");
        return -1;
    }

    // 初始化解码器上下文
    decoder_codec_ctx = avcodec_alloc_context3(decoder_codec);
    if (!decoder_codec_ctx)
    {
        fprintf(stderr, "Failed to allocate codec context\n");
        return -1;
    }
    AVCodecParameters *codecpar = ifmt_ctx->streams[input_video_stream_index]->codecpar;
    // 将解码器参数拷贝到解码器上下文
    if (avcodec_parameters_to_context(decoder_codec_ctx, codecpar) < 0)
    {
        fprintf(stderr, "Failed to copy codec parameters to decoder context\n");
        return -1;
    }

    // 初始化编码器上下文
    AVCodecContext *encoder_codec_ctx = avcodec_alloc_context3(encoder_codec);
    if (!encoder_codec_ctx)
    {
        fprintf(stderr, "Could not allocate video codec context\n");
        return -1;
    }
    // 设置编码器参数
    encoder_codec_ctx->time_base = ifmt_ctx->streams[input_video_stream_index]->time_base;
    encoder_codec_ctx->sample_aspect_ratio = decoder_codec_ctx->sample_aspect_ratio;
    encoder_codec_ctx->pix_fmt = decoder_codec_ctx->pix_fmt;
    // may be need to change
    encoder_codec_ctx->width = decoder_codec_ctx->width;
    encoder_codec_ctx->height = decoder_codec_ctx->height;
    encoder_codec_ctx->framerate = decoder_codec_ctx->framerate;
    encoder_codec_ctx->bit_rate = decoder_codec_ctx->bit_rate;

    if (avcodec_open2(decoder_codec_ctx, decoder_codec, nullptr) < 0)
    {
        fprintf(stderr, "Failed to open decodec\n");
        return -1;
    }

    if (avcodec_open2(encoder_codec_ctx, encoder_codec, nullptr) < 0)
    {
        fprintf(stderr, "Failed to open encodec\n");
        return -1;
    }

    // 添加视频流
    AVStream* output_video_stream = avformat_new_stream(ofmt_ctx, encoder_codec);
    if (!output_video_stream)
    {
        fprintf(stderr, "Could not create video stream\n");
        return -1;
    }
    if (avcodec_parameters_from_context(output_video_stream->codecpar, encoder_codec_ctx) < 0)
    {
        fprintf(stderr, "Failed to copy codec parameters to output stream\n");
        return -1;
    }
    output_video_stream->time_base = encoder_codec_ctx->time_base;

    av_dump_format(ofmt_ctx, 0, output_filename, 1);

    // 打开输出文件
    if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
    {
        if (avio_open(&ofmt_ctx->pb, output_filename, AVIO_FLAG_WRITE) < 0)
        {
            fprintf(stderr, "Could not open output file '%s'\n", output_filename);
            return -1;
        }
    }
    // 写文件头
    if (avformat_write_header(ofmt_ctx, nullptr) < 0)
    {
        fprintf(stderr, "Error occurred when opening output file\n");
        return -1;
    }

    // 读取视频帧
    struct SwsContext *sws_before_ctx = sws_getContext(decoder_codec_ctx->width, decoder_codec_ctx->height,
                                                       AV_PIX_FMT_YUV420P,
                                                       decoder_codec_ctx->width, decoder_codec_ctx->height,
                                                       AV_PIX_FMT_RGB24,
                                                       SWS_BILINEAR, NULL, NULL, NULL);
    struct SwsContext *sws_after_ctx = sws_getContext(encoder_codec_ctx->width, encoder_codec_ctx->height,
                                                      AV_PIX_FMT_RGB24,
                                                      encoder_codec_ctx->width, encoder_codec_ctx->height, 
                                                      AV_PIX_FMT_YUV420P,
                                                      SWS_BILINEAR, NULL, NULL, NULL);

    if (!sws_before_ctx || !sws_after_ctx)
    {
        fprintf(stderr, "Could not initialize the conversion context\n");
        return -1;
    }

    input_frame = av_frame_alloc();
    output_frame = av_frame_alloc();
    if (!output_frame || !input_frame)
    {
        fprintf(stderr, "Could not allocate video frame\n");
        return -1;
    }

    output_frame->format = encoder_codec_ctx->pix_fmt;
    output_frame->width = encoder_codec_ctx->width;
    output_frame->height = encoder_codec_ctx->height;

    if (av_frame_get_buffer(output_frame, 32) < 0)
    {
        fprintf(stderr, "Could not allocate the video frame data\n");
        return -1;
    }

    inpkt = av_packet_alloc();
    outpkt = av_packet_alloc();
    if (!inpkt || !outpkt)
    {
        fprintf(stderr, "Could not allocate the video packet\n");
        return -1;
    }

    progresscpp::ProgressBar progressBar("processing video:", ifmt_ctx->streams[input_video_stream_index]->nb_frames, 70);

    while (av_read_frame(ifmt_ctx, inpkt) >= 0)
    {
        if (inpkt->stream_index == input_video_stream_index)
        {
            // 处理视频帧
            if (avcodec_send_packet(decoder_codec_ctx, inpkt) == 0)
            {
                // send 和 receive 不是一一对应，可能会发送多帧才能recieve一帧，
                // 也可能发送一帧就能接收多帧，所以必须循环接收
                while (avcodec_receive_frame(decoder_codec_ctx, input_frame) == 0)
                {
                    // 将帧保存为图片
                    ncnn::Mat img(input_frame->width, input_frame->height, 3, (size_t)1);  // RGB24 elemsize=3u, elempack=3
                    uint8_t *indata[1] = {(uint8_t *)img.data};
                    int inlinesize = img.w * 3;
                    sws_scale(sws_before_ctx, input_frame->data, input_frame->linesize, 0, 
                                input_frame->height, indata, &inlinesize);
                    // // 处理图片
                    // // process(img);

                    ncnn::Mat predimg = img.clone();

                    // // 将图片保存为帧
                    uint8_t *outdata[1] = {(uint8_t *)predimg.data};
                    int line_size = predimg.w * 3;
                    sws_scale(sws_after_ctx, outdata, &line_size, 0, predimg.h, output_frame->data, output_frame->linesize);

                    output_frame->pts = input_frame->pts;
                    output_frame->pkt_dts = input_frame->pkt_dts;
                    output_frame->duration = input_frame->duration;

                    // 编码帧
                    if (avcodec_send_frame(encoder_codec_ctx, output_frame) < 0)
                    {
                        fprintf(stderr, "Error sending a frame for encoding\n");
                        return -1;
                    }
                    while (avcodec_receive_packet(encoder_codec_ctx, outpkt) >= 0)
                    {
                        ++progressBar;
                        outpkt->stream_index = output_video_stream->index;
                        av_packet_rescale_ts(outpkt, ifmt_ctx->streams[input_video_stream_index]->time_base,
                                             output_video_stream->time_base);
                        if (av_interleaved_write_frame(ofmt_ctx, outpkt) < 0)
                        {
                            fprintf(stderr, "Error while writing output packet\n");
                            return -1;
                        }
                    }
                }              
            }
            else
            {
                fprintf(stderr, "Error sending a packet to the decoder\n");
                return -1;
            }
                        
        }
        else
        {
            inpkt->stream_index = streams_mapping[inpkt->stream_index];
            if (av_interleaved_write_frame(ofmt_ctx, inpkt) < 0)
            {
                fprintf(stderr, "Error while writing output packet\n");
                return -1;
            }
        }
        progressBar.display();
        av_packet_unref(inpkt); // decoder not free the packet before use it
    }

    // 写文件尾
    if (av_write_trailer(ofmt_ctx) < 0)
    {
        fprintf(stderr, "Error occurred when writing trailer\n");
        return -1;
    }
    progressBar.done();

    // 释放资源
    sws_freeContext(sws_before_ctx);
    sws_freeContext(sws_after_ctx);
    av_frame_free(&input_frame);
    av_frame_free(&output_frame);
    av_packet_free(&inpkt);
    av_packet_free(&outpkt);
    avcodec_free_context(&decoder_codec_ctx);
    avcodec_free_context(&encoder_codec_ctx);
    avformat_close_input(&ofmt_ctx);
    if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
    {
        avio_closep(&ofmt_ctx->pb);
    }
    avformat_free_context(ofmt_ctx);
    av_free(new_streams);
    av_free(streams_mapping);
    return 0;
}