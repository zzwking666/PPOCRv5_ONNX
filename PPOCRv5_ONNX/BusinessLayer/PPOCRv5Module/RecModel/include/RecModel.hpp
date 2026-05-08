#pragma once
#include <onnxruntime_cxx_api.h>
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <memory>

class RecModel {
public:
    RecModel();
    ~RecModel();

    bool Init(const std::string& model_path, const std::string& dict_path,
        bool use_gpu = false);
    std::pair<std::string, float> Infer(const cv::Mat& image);

private:
    Ort::Env env_;
    Ort::SessionOptions session_options_;
    std::unique_ptr<Ort::Session> session_;
    Ort::MemoryInfo memory_info_;

    std::vector<const char*> input_names_;
    std::vector<const char*> output_names_;

    std::vector<std::string> dict_;
    int blank_idx_ = 0;

    // 预处理参数
    const int rec_image_height_ = 48;
    const int rec_batch_num_ = 6;
    const int rec_max_len_ = 25;

    cv::Mat Preprocess(const cv::Mat& image, int& dst_width);
    std::string Postprocess(const std::vector<float>& output, int seq_len, float& score);
};