#include "Postprocess.hpp"
#include "DetModel.hpp"
#include <opencv2/imgproc.hpp>
#include <algorithm>
#include <cmath>

namespace Postprocess {

    std::vector<TextBox> DBPostprocess(const cv::Mat& pred, float thresh,
        float box_thresh, float unclip_ratio,
        const cv::Size& ori_size, float scale) {
        // 二值化
        cv::Mat binary;
        cv::threshold(pred, binary, thresh * 255, 255, cv::THRESH_BINARY);
        binary.convertTo(binary, CV_8U);

        // 查找轮廓
        std::vector<std::vector<cv::Point>> contours;
        std::vector<cv::Vec4i> hierarchy;
        cv::findContours(binary, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

        std::vector<TextBox> boxes;
        for (auto& contour : contours) {
            float area = cv::contourArea(contour);
            if (area < 10) continue;  // 过滤太小的区域

            // 最小外接矩形
            cv::RotatedRect min_rect = cv::minAreaRect(contour);
            float rect_area = min_rect.size.width * min_rect.size.height;

            if (rect_area < 10) continue;

            // 计算 box score
            cv::Mat box_mask = cv::Mat::zeros(pred.size(), CV_8U);
            std::vector<std::vector<cv::Point>> tmp = { contour };
            cv::drawContours(box_mask, tmp, 0, cv::Scalar(255), -1);

            cv::Mat box_pred;
            pred.copyTo(box_pred, box_mask);
            float score = static_cast<float>(cv::sum(box_pred)[0]) / (area + 1e-6);

            if (score < box_thresh) continue;

            // 获取4个角点
            cv::Point2f vertices[4];
            min_rect.points(vertices);

            std::vector<cv::Point> box;
            for (int i = 0; i < 4; i++) {
                box.push_back(cv::Point(static_cast<int>(vertices[i].x / scale),
                    static_cast<int>(vertices[i].y / scale)));
            }

            // Unclip 膨胀
            box = Unclip(box, unclip_ratio);

            // 裁剪到图像范围内
            for (auto& pt : box) {
                pt.x = std::max(0, std::min(pt.x, ori_size.width - 1));
                pt.y = std::max(0, std::min(pt.y, ori_size.height - 1));
            }

            boxes.push_back({ box, score });
        }

        // NMS 去重
        std::sort(boxes.begin(), boxes.end(), [](const TextBox& a, const TextBox& b) {
            return a.score > b.score;
            });

        std::vector<TextBox> result;
        std::vector<bool> suppressed(boxes.size(), false);

        for (size_t i = 0; i < boxes.size(); i++) {
            if (suppressed[i]) continue;
            result.push_back(boxes[i]);

            for (size_t j = i + 1; j < boxes.size(); j++) {
                if (suppressed[j]) continue;
                // 简单的IOU判断
                float iou = 0.0f;  // 简化处理，实际需要计算多边形IOU
                if (iou > 0.5f) suppressed[j] = true;
            }
        }

        return result;
    }

    std::vector<cv::Point> Unclip(const std::vector<cv::Point>& box, float unclip_ratio) {
        // 使用 shapely 或 OpenCV 实现多边形膨胀
        // 简化版本：按比例扩大
        cv::Point center(0, 0);
        for (const auto& pt : box) {
            center.x += pt.x;
            center.y += pt.y;
        }
        center.x /= static_cast<int>(box.size());
        center.y /= static_cast<int>(box.size());

        std::vector<cv::Point> expanded;
        for (const auto& pt : box) {
            cv::Point vec(pt.x - center.x, pt.y - center.y);
            expanded.push_back(cv::Point(
                static_cast<int>(center.x + vec.x * unclip_ratio),
                static_cast<int>(center.y + vec.y * unclip_ratio)
            ));
        }

        return expanded;
    }

    float PolygonArea(const std::vector<cv::Point>& poly) {
        return static_cast<float>(cv::contourArea(poly));
    }

    std::vector<TextBox> FilterBoxes(const std::vector<TextBox>& boxes, float min_size) {
        std::vector<TextBox> result;
        for (const auto& box : boxes) {
            float w = std::abs(box.points[0].x - box.points[2].x);
            float h = std::abs(box.points[0].y - box.points[2].y);
            if (w > min_size && h > min_size) {
                result.push_back(box);
            }
        }
        return result;
    }

    std::string CTCDecode(const std::vector<int>& preds,
        const std::vector<std::string>& dict, int blank_idx) {
        std::string result;
        int prev = -1;

        for (int pred : preds) {
            if (pred != blank_idx && pred != prev) {
                if (pred >= 0 && pred < static_cast<int>(dict.size())) {
                    result += dict[pred];
                }
            }
            prev = pred;
        }

        return result;
    }

} // namespace Postprocess