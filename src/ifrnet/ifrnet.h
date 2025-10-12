#ifndef IFRNET_H
#define IFRNET_H

#include <string>

// ncnn
#include "net.h"

/**
 * @class IFRNet
 * @brief The IFRNet class provides an interface for video frame interpolation using the IFRNet model.
 */
class IFRNet
{
public:
    /**
     * @brief Construct a new IFRNet object.
     * @param gpuid The GPU device ID to use.
     * @param model_path_dir The directory path to the IFRNet model files.
     * @param scale The interpolation scale factor.
     * @param tta_mode Whether to use test-time augmentation.
     * @param num_threads The number of threads to use for processing.
     * @param type The model type.
     * @param GoProversion Whether to use the GoPro version of the model.
     */
    IFRNet(int gpuid, std::string model_path_dir, int scale, bool tta_mode = false, 
            int num_threads = 1, int type=1, bool GoProversion=false);

    /**
     * @brief Destroy the IFRNet object.
     */
    ~IFRNet();

    /**
     * @brief Load the IFRNet model.
     * @param modeldir The directory path to the IFRNet model files.
     * @return int 0 on success, -1 on failure.
     */
    int load(const std::string& modeldir);

    /**
     * @brief Process two input images to generate an interpolated frame.
     * @param in0image The first input image.
     * @param in1image The second input image.
     * @param timestep The interpolation timestep.
     * @param outimage The output interpolated image.
     * @return int 0 on success, -1 on failure.
     */
    int process(const ncnn::Mat& in0image, const ncnn::Mat& in1image, float timestep, ncnn::Mat& outimage) const;

    /**
     * @brief Process two input images on the CPU to generate an interpolated frame.
     * @param in0image The first input image.
     * @param in1image The second input image.
     * @param timestep The interpolation timestep.
     * @param outimage The output interpolated image.
     * @return int 0 on success, -1 on failure.
     */
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
