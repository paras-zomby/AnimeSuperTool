// rife implemented with ncnn library

#ifndef RIFE_H
#define RIFE_H

#include <string>

// ncnn
#include "net.h"

/**
 * @class RIFE
 * @brief The RIFE class provides an interface for video frame interpolation using the RIFE model.
 */
class RIFE
{
public:
    /**
     * @brief Construct a new RIFE object.
     * @param gpuid The GPU device ID to use.
     * @param model_path_dir The directory path to the RIFE model files.
     * @param scale The interpolation scale factor.
     * @param tta_mode Whether to use test-time augmentation.
     * @param tta_temporal_mode Whether to use temporal test-time augmentation.
     * @param uhd_mode Whether to use UHD mode.
     * @param num_threads The number of threads to use for processing.
     * @param rife_v2 Whether to use RIFE v2 model.
     * @param rife_v4 Whether to use RIFE v4 model.
     */
    RIFE(int gpuid, std::string model_path_dir, int scale, bool tta_mode = false, 
            bool tta_temporal_mode = false, bool uhd_mode = false, 
            int num_threads = 1, bool rife_v2 = false, bool rife_v4 = false);
    /**
     * @brief Destroy the RIFE object.
     */
    ~RIFE();

    /**
     * @brief Load the RIFE model.
     * @param modeldir The directory path to the RIFE model files.
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

    /**
     * @brief Process two input images using the RIFE v4 model.
     * @param in0image The first input image.
     * @param in1image The second input image.
     * @param timestep The interpolation timestep.
     * @param outimage The output interpolated image.
     * @return int 0 on success, -1 on failure.
     */
    int process_v4(const ncnn::Mat& in0image, const ncnn::Mat& in1image, float timestep, ncnn::Mat& outimage) const;

    /**
     * @brief Process two input images on the CPU using the RIFE v4 model.
     * @param in0image The first input image.
     * @param in1image The second input image.
     * @param timestep The interpolation timestep.
     * @param outimage The output interpolated image.
     * @return int 0 on success, -1 on failure.
     */
    int process_v4_cpu(const ncnn::Mat& in0image, const ncnn::Mat& in1image, float timestep, ncnn::Mat& outimage) const;

private:
    ncnn::VulkanDevice* vkdev;
    ncnn::Net flownet;
    ncnn::Net contextnet;
    ncnn::Net fusionnet;
    ncnn::Pipeline* rife_preproc;
    ncnn::Pipeline* rife_postproc;
    ncnn::Pipeline* rife_flow_tta_avg;
    ncnn::Pipeline* rife_flow_tta_temporal_avg;
    ncnn::Pipeline* rife_out_tta_temporal_avg;
    ncnn::Pipeline* rife_v4_timestep;
    ncnn::Layer* rife_uhd_downscale_image;
    ncnn::Layer* rife_uhd_upscale_flow;
    ncnn::Layer* rife_uhd_double_flow;
    ncnn::Layer* rife_v2_slice_flow;
    bool tta_mode;
    bool tta_temporal_mode;
    bool uhd_mode;
    int num_threads;
    bool rife_v2;
    bool rife_v4;
};

#endif // RIFE_H
