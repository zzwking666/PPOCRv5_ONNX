#pragma once
#include <onnxruntime_cxx_api.h>
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <memory>

struct TextBox {
    std::vector<cv::Point> points;  // 4个角点
    float score;
};

class DetModel {
public:
    DetModel();
    ~DetModel();

    bool Init(const std::string& model_path, bool use_gpu = false);
    std::vector<TextBox> Infer(const cv::Mat& image);

private:
    Ort::Env env_;
    Ort::SessionOptions session_options_;
    std::unique_ptr<Ort::Session> session_;
    Ort::MemoryInfo memory_info_;

    std::vector<const char*> input_names_;
    std::vector<const char*> output_names_;
    std::vector<int64_t> input_shape_;

    // 预处理参数（从 inference.yml 读取或硬编码）
    const int target_size_ = 960;      // 检测模型输入尺寸（长边）
    const float det_db_thresh_ = 0.3f;   // DB 二值化阈值
    const float det_db_box_thresh_ = 0.6f;
    const float det_db_unclip_ratio_ = 1.5f;
    const bool use_dilation_ = false;

    cv::Mat Preprocess(const cv::Mat& image, float& scale);
    std::vector<TextBox> Postprocess(const cv::Mat& pred, float scale,
        const cv::Size& ori_size);
};