#pragma once
#include <opencv2/opencv.hpp>
#include <vector>

struct TextBox;

namespace Postprocess {
    // DBNet 后处理：从概率图提取文本框
    std::vector<TextBox> DBPostprocess(const cv::Mat& pred, float thresh,
        float box_thresh, float unclip_ratio,
        const cv::Size& ori_size, float scale);

    // 膨胀/收缩多边形
    std::vector<cv::Point> Unclip(const std::vector<cv::Point>& box, float unclip_ratio);

    // 计算多边形面积
    float PolygonArea(const std::vector<cv::Point>& poly);

    // 按得分排序并过滤小框
    std::vector<TextBox> FilterBoxes(const std::vector<TextBox>& boxes,
        float min_size = 3.0f);

    // CTC 解码
    std::string CTCDecode(const std::vector<int>& preds,
        const std::vector<std::string>& dict, int blank_idx);
}