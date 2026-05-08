#pragma once
#include <opencv2/opencv.hpp>

namespace Preprocess {
    // 检测模型预处理：Resize + Normalize + HWC->CHW
    cv::Mat DetPreprocess(const cv::Mat& image, int target_size, float& scale);

    // 识别模型预处理：高度固定48，宽度动态调整
    cv::Mat RecPreprocess(const cv::Mat& image, int target_height, int& dst_width);

    // 归一化并转为CHW格式
    std::vector<float> NormalizeCHW(const cv::Mat& image,
        const std::vector<float>& mean,
        const std::vector<float>& scale);
}