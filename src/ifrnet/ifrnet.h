#ifndef IFRNET_H
#define IFRNET_H

#include <string>

// ncnn
#include "net.h"

class IFRNet
{
public:
    IFRNet(int gpuid, std::string model_path_dir, int scale, bool tta_mode = false, 
            int num_threads = 1, int type=1, bool GoProversion=false);
    ~IFRNet();

    int load(const std::string& modeldir);

    int process(const ncnn::Mat& in0image, const ncnn::Mat& in1image, float timestep, ncnn::Mat& outimage) const;

    int process_cpu(const ncnn::Mat& in0image, const ncnn::Mat& in1image, float timestep, ncnn::Mat& outimage) const;

private:
    ncnn::VulkanDevice* vkdev;
    ncnn::Net ifrnet;
    ncnn::Pipeline* ifrnet_preproc;
    ncnn::Pipeline* ifrnet_postproc;
    ncnn::Pipeline* ifrnet_timestep;
    bool tta_mode;
    int num_threads;
};

#endif // IFRNET_H
