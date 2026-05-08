#pragma once
#include <onnxruntime_cxx_api.h>
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <memory>
#include "ocr_utils.hpp"  // 只依赖 Utils

class DetModel {
public:
    struct Config {
        int target_size = 960;
        float db_thresh = 0.1f;
        float db_box_thresh = 0.1f;
        float db_unclip_ratio = 1.5f;
    };

    DetModel();
    ~DetModel();

    bool Init(const std::string& model_path, const Config& cfg = {}, bool use_gpu = false);

    // 原始推理：返回概率图，调用方自己做后处理
    cv::Mat InferRaw(const cv::Mat& image, float& scale);

    // 便捷接口：内部调用 utils 做后处理
    std::vector<ocr_utils::TextBox> Infer(const cv::Mat& image);

private:
    Ort::Env env_;
    Ort::SessionOptions session_options_;
    std::unique_ptr<Ort::Session> session_;
    Ort::MemoryInfo memory_info_;
    Config cfg_;

    std::vector<std::string> input_name_storage_;
    std::vector<std::string> output_name_storage_;
    std::vector<const char*> input_names_;
    std::vector<const char*> output_names_;
};