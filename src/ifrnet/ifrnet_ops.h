// ifrnet implemented with ncnn library

#ifndef IFRNET_OPS_H
#define IFRNET_OPS_H

#include <vector>

// ncnn
#include "layer.h"
#include "pipeline.h"

class IFRNetWarp : public ncnn::Layer
{
public:
    IFRNetWarp();
    virtual int create_pipeline(const ncnn::Option& opt);
    virtual int destroy_pipeline(const ncnn::Option& opt);
    virtual int forward(const std::vector<ncnn::Mat>& bottom_blobs, std::vector<ncnn::Mat>& top_blobs, const ncnn::Option& opt) const;
    virtual int forward(const std::vector<ncnn::VkMat>& bottom_blobs, std::vector<ncnn::VkMat>& top_blobs, ncnn::VkCompute& cmd, const ncnn::Option& opt) const;

private:
    ncnn::Pipeline* pipeline_warp;
    ncnn::Pipeline* pipeline_warp_pack4;
    ncnn::Pipeline* pipeline_warp_pack8;
};

#endif // IFRNET_OPS_H
