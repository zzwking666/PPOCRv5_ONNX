#pragma once
#include <onnxruntime_cxx_api.h>
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <memory>
#include "ocr_utils.hpp"

class RecModel {
public:
    struct Config {
        int rec_image_height = 48;
    };

    RecModel();
    ~RecModel();

    bool Init(const std::string& model_path, const std::string& dict_path,
        const Config& cfg = {}, bool use_gpu = false);

    // 原始推理：返回 logits
    std::vector<float> InferRaw(const cv::Mat& image, int& seq_len);

    // 便捷接口
    std::pair<std::string, float> Infer(const cv::Mat& image);

private:
    Ort::Env env_;
    Ort::SessionOptions session_options_;
    std::unique_ptr<Ort::Session> session_;
    Ort::MemoryInfo memory_info_;
    Config cfg_;

    std::vector<std::string> dict_;
    int blank_idx_ = 0;

    std::vector<std::string> input_name_storage_;
    std::vector<std::string> output_name_storage_;
    std::vector<const char*> input_names_;
    std::vector<const char*> output_names_;
};