// realcugan implemented with ncnn library

#ifndef REALCUGAN_H
#define REALCUGAN_H

#include <string>

// ncnn
#include "net.h"
#include "gpu.h"
#include "layer.h"

class FeatureCache;
/**
 * @class RealCUGAN
 * @brief The RealCUGAN class provides an interface for image super-resolution using the Real-CUGAN model.
 */
class RealCUGAN
{
public:
    /**
     * @brief Construct a new RealCUGAN object.
     * @param gpuid The GPU device ID to use.
     * @param model_path_dir The directory path to the Real-CUGAN model files.
     * @param _tilenum The number of tiles to use for processing.
     * @param zoom_scale The zoom scale factor.
     * @param num_threads The number of threads to use for processing.
     * @param denoise Whether to perform denoising.
     * @param _syncgap The sync gap value.
     * @param _tta_mode Whether to use test-time augmentation.
     */
    RealCUGAN(int gpuid, std::string model_path_dir, int _tilenum, int zoom_scale = 2, 
                int num_threads = 1, bool denoise = false, int _syncgap = 0, bool _tta_mode = false);
    /**
     * @brief Destroy the RealCUGAN object.
     */
    ~RealCUGAN();

    /**
     * @brief Load the Real-CUGAN model.
     * @param parampath The path to the model parameter file.
     * @param modelpath The path to the model binary file.
     * @return int 0 on success, -1 on failure.
     */
    int load(const std::string& parampath, const std::string& modelpath);

    /**
     * @brief Process an input image to generate a super-resolved output image.
     * @param inimage The input image.
     * @param outimage The output super-resolved image.
     * @return int 0 on success, -1 on failure.
     */
    int process(const ncnn::Mat& inimage, ncnn::Mat& outimage) const;

    /**
     * @brief Process an input image on the CPU to generate a super-resolved output image.
     * @param inimage The input image.
     * @param outimage The output super-resolved image.
     * @return int 0 on success, -1 on failure.
     */
    int process_cpu(const ncnn::Mat& inimage, ncnn::Mat& outimage) const;

    /**
     * @brief Process an input image using the 'se' variant of the model.
     * @param inimage The input image.
     * @param outimage The output super-resolved image.
     * @return int 0 on success, -1 on failure.
     */
    int process_se(const ncnn::Mat& inimage, ncnn::Mat& outimage) const;

    /**
     * @brief Process an input image on the CPU using the 'se' variant of the model.
     * @param inimage The input image.
     * @param outimage The output super-resolved image.
     * @return int 0 on success, -1 on failure.
     */
    int process_cpu_se(const ncnn::Mat& inimage, ncnn::Mat& outimage) const;

    /**
     * @brief Process an input image using the 'se_rough' variant of the model.
     * @param inimage The input image.
     * @param outimage The output super-resolved image.
     * @return int 0 on success, -1 on failure.
     */
    int process_se_rough(const ncnn::Mat& inimage, ncnn::Mat& outimage) const;

    /**
     * @brief Process an input image on the CPU using the 'se_rough' variant of the model.
     * @param inimage The input image.
     * @param outimage The output super-resolved image.
     * @return int 0 on success, -1 on failure.
     */
    int process_cpu_se_rough(const ncnn::Mat& inimage, ncnn::Mat& outimage) const;

    /**
     * @brief Process an input image using the 'se_very_rough' variant of the model.
     * @param inimage The input image.
     * @param outimage The output super-resolved image.
     * @return int 0 on success, -1 on failure.
     */
    int process_se_very_rough(const ncnn::Mat& inimage, ncnn::Mat& outimage) const;

    /**
     * @brief Process an input image on the CPU using the 'se_very_rough' variant of the model.
     * @param inimage The input image.
     * @param outimage The output super-resolved image.
     * @return int 0 on success, -1 on failure.
     */
    int process_cpu_se_very_rough(const ncnn::Mat& inimage, ncnn::Mat& outimage) const;

protected:
    int process_se_stage0(const ncnn::Mat& inimage, const std::vector<std::string>& names, const std::vector<std::string>& outnames, const ncnn::Option& opt, FeatureCache& cache) const;
    int process_se_stage2(const ncnn::Mat& inimage, const std::vector<std::string>& names, ncnn::Mat& outimage, const ncnn::Option& opt, FeatureCache& cache) const;
    int process_se_sync_gap(const ncnn::Mat& inimage, const std::vector<std::string>& names, const ncnn::Option& opt, FeatureCache& cache) const;

    int process_se_very_rough_stage0(const ncnn::Mat& inimage, const std::vector<std::string>& names, const std::vector<std::string>& outnames, const ncnn::Option& opt, FeatureCache& cache) const;
    int process_se_very_rough_sync_gap(const ncnn::Mat& inimage, const std::vector<std::string>& names, const ncnn::Option& opt, FeatureCache& cache) const;

    int process_cpu_se_stage0(const ncnn::Mat& inimage, const std::vector<std::string>& names, const std::vector<std::string>& outnames, FeatureCache& cache) const;
    int process_cpu_se_stage2(const ncnn::Mat& inimage, const std::vector<std::string>& names, ncnn::Mat& outimage, FeatureCache& cache) const;
    int process_cpu_se_sync_gap(const ncnn::Mat& inimage, const std::vector<std::string>& names, FeatureCache& cache) const;

    int process_cpu_se_very_rough_stage0(const ncnn::Mat& inimage, const std::vector<std::string>& names, const std::vector<std::string>& outnames, FeatureCache& cache) const;
    int process_cpu_se_very_rough_sync_gap(const ncnn::Mat& inimage, const std::vector<std::string>& names, FeatureCache& cache) const;

public:
    // realcugan parameters
    int noise;
    int scale;
    int tilenum;
    int prepadding;
    int syncgap;

private:
    ncnn::VulkanDevice* vkdev;
    ncnn::Net net;
    ncnn::Pipeline* realcugan_preproc;
    ncnn::Pipeline* realcugan_postproc;
    ncnn::Pipeline* realcugan_4x_postproc;
    ncnn::Layer* bicubic_2x;
    ncnn::Layer* bicubic_3x;
    ncnn::Layer* bicubic_4x;
    bool tta_mode;
};

#endif // REALCUGAN_H
