#pragma once
#include "DetModel.hpp"
#include "RecModel.hpp"
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

struct OCRResult {
    std::vector<cv::Point> box;
    std::string text;
    float score;
};

class PPOCRv5System {
public:
    PPOCRv5System();
    ~PPOCRv5System();

    bool Init(const std::string& det_model_path,
        const std::string& rec_model_path,
        const std::string& dict_path,
        bool use_gpu = false);

    std::vector<OCRResult> OCR(const cv::Mat& image);

private:
    DetModel det_model_;
    RecModel rec_model_;

    cv::Mat CropBox(const cv::Mat& image, const std::vector<cv::Point>& box);
    std::vector<cv::Point> OrderPoints(const std::vector<cv::Point>& pts);
};