#pragma once
#include "DetModel.hpp"
#include "RecModel.hpp"
#include "ocr_utils.hpp"
#include <opencv2/opencv.hpp>
#include <vector>

class PPOCRv5System {
public:
    PPOCRv5System();
    ~PPOCRv5System();

    bool Init(const std::string& det_model_path,
        const std::string& rec_model_path,
        const std::string& dict_path,
        bool use_gpu = false);

    std::vector<ocr_utils::OCRResult> OCR(const cv::Mat& image);

private:
    DetModel det_;
    RecModel rec_;
};