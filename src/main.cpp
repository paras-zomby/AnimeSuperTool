#include "net.h"
#include "utils/config.h"
#include "utils/progressbar.hpp"
#include "utils/cyclebuffer.hpp"
#include "videoutils/videoreader.h"
#include "videoutils/videowriter.h"
#include "rife/rife.h"
#include "ifrnet/ifrnet.h"
#include "realcugan/realcugan.h"
#include "boost/filesystem.hpp"

bool workflow(const Params &param, const std::string& filename, const std::string& output_filename)
{
    VideoReader videoReader;
    VideoWriter videoWriter;

    IFRNet ifrnet(param.gpu, "../models", param.zoom_scale, false, 
                    param.num_threads_pergpu, param.ifrnet_type, false);
    RealCUGAN realcugan(param.gpu, "../models/realcugan-pro", param.tile_num, 
                        param.zoom_scale, param.num_threads_pergpu, 
                        param.denoise, 0, false);
    
    if (!videoReader.open_video(filename))
    {
        fprintf(stderr, "Failed to open video file, error: %s\n", videoReader.get_error_string().c_str());
        return -1;
    }
    if (!videoWriter.open_video_file(output_filename))
    {
        fprintf(stderr, "Failed to open output file, error: %s\n", videoWriter.get_error_string().c_str());
        return -1;
    }
    if (!videoWriter.set_params(videoReader, param))
    {
        fprintf(stderr, "Failed to set video params, error: %s\n", videoWriter.get_error_string().c_str());
        return -1;
    }
    if (!videoWriter.init_video())
    {
        fprintf(stderr, "Failed to init video, error: %s\n", videoWriter.get_error_string().c_str());
        return -1;
    }

    CycleBuffer<Frame, 4> buffer(param.frame_ratio_scale - 1);
    bool flag = false;
    progresscpp::ProgressBar progressBar("Video remuxing: ", videoReader.get_video_stream()->nb_frames, 100);
    while (1)
    {
        // TODO: 不能区分是错误还是结束
        if(!videoReader.read_frame(buffer.tail()))
            break;
        if (buffer.tail().type == FrameType::VIDEO)
        {
            if (!flag)  // need two video frames to start processing
            {
                flag = true;
                buffer.update();
                continue;
            }
            // Frame Interpolation
            for (size_t i = 1; i < param.frame_ratio_scale; ++i)
                ifrnet.process(buffer.head().img, buffer.tail().img, i * 1.0 / param.frame_ratio_scale, buffer[i].img);
            fix_timestamp(buffer, param.frame_ratio_scale);
            // Frame Upscaling and write to video
            for (size_t i = 0; i < param.frame_ratio_scale; ++i)
            {
                ncnn::Mat out;
                realcugan.process(buffer[i].img, out);
                buffer[i].img = out;
                if(!videoWriter.write_frame(buffer[i]))
                {
                    fprintf(stderr, "Failed to write frame, error: %s\n", videoWriter.get_error_string().c_str());
                    return -1;
                }
            }
            buffer.update();
            ++progressBar;
            progressBar.display();
        }
        else if(!videoWriter.write_frame(buffer.tail()))
        {
            fprintf(stderr, "Failed to write frame, error: %s\n", videoWriter.get_error_string().c_str());
            return -1;
        }
        else if (buffer.tail().type == FrameType::FIN)
        {
            break;
        }
    }
    // write last frame
    fix_duration(buffer.head(), param.frame_ratio_scale);
    ncnn::Mat out;
    realcugan.process(buffer.head().img, out);
    buffer.head().img = out;
    videoWriter.write_frame(buffer.head());
    progressBar.done();
    return true;
}

int main() {

    // configs
    Params param;
    param.zoom_scale = 2;
    param.frame_ratio_scale = 3;
    param.denoise = false;
    param.num_threads_pergpu = 2;
    param.tile_num = 1;
    param.gpu = 1;
    param.ifrnet_type = 2;
    param.imagezoom_bitrate_scale = 0.7;
    param.framerate_bitrate_scale = 0.5;

    std::stringstream discription;
    discription << "[AST:" << param.zoom_scale << "x" << param.frame_ratio_scale << "x]";

    boost::filesystem::path video_path("../video_small.mp4");
    if (!boost::filesystem::exists(video_path))
    {
        fprintf(stderr, "Video file not exist: %s\n", video_path.c_str());
        return -1;
    }
    if (boost::filesystem::is_directory(video_path))
    {
        for (boost::filesystem::directory_entry& x : boost::filesystem::directory_iterator(video_path))
        {
            std::string filename = x.path().string();
            std::string output_filename = x.path().string();
            output_filename.insert(output_filename.rfind("."), discription.str());
            if (!workflow(param, filename, output_filename))
            {
                fprintf(stderr, "Failed to process video file: %s\n", filename.c_str());
                return -1;
            }
        }
    }
    else
    {
        std::string filename = video_path.string();
        std::string output_filename = video_path.string();
        output_filename.insert(output_filename.rfind("."), discription.str());
        if (!workflow(param, filename, output_filename))
        {
            fprintf(stderr, "Failed to process video file: %s\n", filename.c_str());
            return -1;
        }
    }
    return 0;
}