#include "PPOCRv5System.hpp"
#include <iostream>

PPOCRv5System::PPOCRv5System() = default;
PPOCRv5System::~PPOCRv5System() = default;

bool PPOCRv5System::Init(const std::string& det_model_path,
    const std::string& rec_model_path,
    const std::string& dict_path,
    bool use_gpu) {
    if (!det_model_.Init(det_model_path, use_gpu)) {
        std::cerr << "Failed to init det model" << std::endl;
        return false;
    }
    if (!rec_model_.Init(rec_model_path, dict_path, use_gpu)) {
        std::cerr << "Failed to init rec model" << std::endl;
        return false;
    }
    return true;
}

std::vector<OCRResult> PPOCRv5System::OCR(const cv::Mat& image) {
    std::vector<OCRResult> results;

    // 1. 检测
    auto boxes = det_model_.Infer(image);

    // 2. 识别每个文本框
    for (auto& box : boxes) {
        // 裁剪并矫正文本区域
        cv::Mat text_img = CropBox(image, box.points);

        // 识别
        auto [text, score] = rec_model_.Infer(text_img);

        results.push_back({ box.points, text, score });
    }

    return results;
}

cv::Mat PPOCRv5System::CropBox(const cv::Mat& image, const std::vector<cv::Point>& box) {
    // 将倾斜的文本框矫正为水平矩形
    auto ordered = OrderPoints(box);

    float w = std::sqrt(std::pow(ordered[1].x - ordered[0].x, 2) +
        std::pow(ordered[1].y - ordered[0].y, 2));
    float h = std::sqrt(std::pow(ordered[2].x - ordered[1].x, 2) +
        std::pow(ordered[2].y - ordered[1].y, 2));

    cv::Point2f src_pts[4] = {
        cv::Point2f(ordered[0].x, ordered[0].y),
        cv::Point2f(ordered[1].x, ordered[1].y),
        cv::Point2f(ordered[2].x, ordered[2].y),
        cv::Point2f(ordered[3].x, ordered[3].y)
    };

    cv::Point2f dst_pts[4] = {
        cv::Point2f(0, 0),
        cv::Point2f(w, 0),
        cv::Point2f(w, h),
        cv::Point2f(0, h)
    };

    cv::Mat M = cv::getPerspectiveTransform(src_pts, dst_pts);
    cv::Mat dst;
    cv::warpPerspective(image, dst, M, cv::Size(static_cast<int>(w), static_cast<int>(h)));

    return dst;
}

std::vector<cv::Point> PPOCRv5System::OrderPoints(const std::vector<cv::Point>& pts) {
    // 按左上、右上、右下、左下排序
    std::vector<cv::Point> ordered(4);
    std::vector<cv::Point> tmp = pts;

    // 按 y 坐标排序，前两个是上方点
    std::sort(tmp.begin(), tmp.end(), [](const cv::Point& a, const cv::Point& b) {
        return a.y < b.y;
        });

    // 上方两点按 x 排序
    if (tmp[0].x < tmp[1].x) {
        ordered[0] = tmp[0];  // 左上
        ordered[1] = tmp[1];  // 右上
    }
    else {
        ordered[0] = tmp[1];
        ordered[1] = tmp[0];
    }

    // 下方两点按 x 排序
    if (tmp[2].x < tmp[3].x) {
        ordered[3] = tmp[2];  // 左下
        ordered[2] = tmp[3];  // 右下
    }
    else {
        ordered[3] = tmp[3];
        ordered[2] = tmp[2];
    }

    return ordered;
}