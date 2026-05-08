#include "PPOCRv5System.hpp"
#include "ocr_utils.hpp"
#include <iostream>

PPOCRv5System::PPOCRv5System() = default;
PPOCRv5System::~PPOCRv5System() = default;

bool PPOCRv5System::Init(const std::string& det_model_path,
    const std::string& rec_model_path,
    const std::string& dict_path,
    bool use_gpu) {
    if (!det_.Init(det_model_path, {}, use_gpu)) {
        std::cerr << "Failed to init det model" << std::endl;
        return false;
    }
    if (!rec_.Init(rec_model_path, dict_path, {}, use_gpu)) {
        std::cerr << "Failed to init rec model" << std::endl;
        return false;
    }
    return true;
}

std::vector<ocr_utils::OCRResult> PPOCRv5System::OCR(const cv::Mat& image) {
    std::vector<ocr_utils::OCRResult> results;

    auto boxes = det_.Infer(image);

    for (const auto& box : boxes) {
        cv::Mat text_img = ocr_utils::CropAndWarpBox(image, box.points);
        auto [text, score] = rec_.Infer(text_img);

        results.push_back({ box.points, text, score });
    }

    return results;
}