#include "Preprocess.hpp"

namespace Preprocess {

    cv::Mat DetPreprocess(const cv::Mat& image, int target_size, float& scale) {
        int h = image.rows;
        int w = image.cols;

        // 计算缩放比例，长边缩放到 target_size
        scale = std::min(static_cast<float>(target_size) / std::max(h, w), 1.0f);
        int resize_h = static_cast<int>(h * scale);
        int resize_w = static_cast<int>(w * scale);

        // 对齐到32的倍数（PP-OCRv5要求）
        resize_h = std::max((resize_h / 32) * 32, 32);
        resize_w = std::max((resize_w / 32) * 32, 32);

        cv::Mat resized;
        cv::resize(image, resized, cv::Size(resize_w, resize_h));

        // Normalize: [0,255] -> [0,1] 然后减均值除标准差
        cv::Mat float_img;
        resized.convertTo(float_img, CV_32F, 1.0 / 255.0);

        // PP-OCRv5 检测模型 mean=[0.485, 0.456, 0.406], std=[0.229, 0.224, 0.225]
        std::vector<cv::Mat> channels(3);
        cv::split(float_img, channels);

        channels[0] = (channels[0] - 0.485f) / 0.229f;
        channels[1] = (channels[1] - 0.456f) / 0.224f;
        channels[2] = (channels[2] - 0.406f) / 0.225f;

        cv::Mat normalized;
        cv::merge(channels, normalized);

        // HWC -> CHW
        std::vector<cv::Mat> chw_channels;
        cv::split(normalized, chw_channels);

        cv::Mat blob(1, 3 * resize_h * resize_w, CV_32F);
        for (int c = 0; c < 3; c++) {
            memcpy(blob.ptr<float>(0) + c * resize_h * resize_w,
                chw_channels[c].data, resize_h * resize_w * sizeof(float));
        }

        // 转为 4D blob [1, 3, H, W]
        blob = blob.reshape(1, { 1, 3, resize_h, resize_w });

        return blob;
    }

    cv::Mat RecPreprocess(const cv::Mat& image, int target_height, int& dst_width) {
        int h = image.rows;
        int w = image.cols;

        // 高度固定为48，宽度按比例缩放
        float ratio = static_cast<float>(w) / h;
        dst_width = static_cast<int>(target_height * ratio);

        // 宽度对齐到4的倍数
        dst_width = (dst_width + 3) / 4 * 4;
        dst_width = std::max(dst_width, 4);  // 最小宽度4

        cv::Mat resized;
        cv::resize(image, resized, cv::Size(dst_width, target_height));

        // 归一化: [0,255] -> [-1, 1] 或根据模型配置
        cv::Mat float_img;
        resized.convertTo(float_img, CV_32F, 1.0 / 255.0);

        // PP-OCRv5 识别模型通常使用 mean=0.5, std=0.5
        std::vector<cv::Mat> channels(3);
        cv::split(float_img, channels);

        for (int c = 0; c < 3; c++) {
            channels[c] = (channels[c] - 0.5f) / 0.5f;
        }

        cv::Mat normalized;
        cv::merge(channels, normalized);

        // HWC -> CHW
        std::vector<cv::Mat> chw_channels;
        cv::split(normalized, chw_channels);

        cv::Mat blob(1, 3 * target_height * dst_width, CV_32F);
        for (int c = 0; c < 3; c++) {
            memcpy(blob.ptr<float>(0) + c * target_height * dst_width,
                chw_channels[c].data, target_height * dst_width * sizeof(float));
        }

        return blob.reshape(1, { 1, 3, target_height, dst_width });
    }

    std::vector<float> NormalizeCHW(const cv::Mat& image,
        const std::vector<float>& mean,
        const std::vector<float>& scale) {
        // 辅助函数，实际在上面已经内联处理了
        return {};
    }

} // namespace Preprocess