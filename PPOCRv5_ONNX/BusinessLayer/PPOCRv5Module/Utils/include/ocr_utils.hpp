#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

namespace ocr_utils {

    // ========== 数据结构 ==========
    struct TextBox {
        std::vector<cv::Point> points;
        float score;
    };

    struct OCRResult {
        std::vector<cv::Point> box;
        std::string text;
        float score;
    };

    // ========== Preprocess 纯函数 ==========
    cv::Mat DetPreprocess(const cv::Mat& image, int target_size, float& scale);
    cv::Mat RecPreprocess(const cv::Mat& image, int target_height, int& dst_width);

    // ========== Postprocess 纯函数 ==========
    std::vector<TextBox> DBPostprocess(const cv::Mat& pred, float thresh,
        float box_thresh, float unclip_ratio,
        const cv::Size& ori_size, float scale);

    std::vector<cv::Point> UnclipPolygon(const std::vector<cv::Point>& box, float ratio);
    std::string CTCDecode(const std::vector<int>& preds,
        const std::vector<std::string>& dict, int blank_idx);

    // ========== 几何工具 ==========
    std::vector<cv::Point> OrderPointsClockwise(const std::vector<cv::Point>& pts);
    cv::Mat CropAndWarpBox(const cv::Mat& image, const std::vector<cv::Point>& box);
    float PolygonIoU(const std::vector<cv::Point>& poly1,
        const std::vector<cv::Point>& poly2);

} // namespace ocr_utils