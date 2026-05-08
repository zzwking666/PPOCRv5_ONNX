#include "ocr_utils.hpp"
#include <algorithm>
#include <cmath>

namespace ocr_utils {

    // ========== Preprocess ==========

    cv::Mat DetPreprocess(const cv::Mat& image, int target_size, float& scale) {
        int h = image.rows;
        int w = image.cols;

        scale = std::min(static_cast<float>(target_size) / std::max(h, w), 1.0f);
        int resize_h = static_cast<int>(h * scale);
        int resize_w = static_cast<int>(w * scale);

        resize_h = std::max((resize_h / 32) * 32, 32);
        resize_w = std::max((resize_w / 32) * 32, 32);

        cv::Mat resized;
        cv::resize(image, resized, cv::Size(resize_w, resize_h));

        cv::Mat float_img;
        resized.convertTo(float_img, CV_32F, 1.0 / 255.0);

        std::vector<cv::Mat> channels(3);
        cv::split(float_img, channels);

        channels[0] = (channels[0] - 0.485f) / 0.229f;
        channels[1] = (channels[1] - 0.456f) / 0.224f;
        channels[2] = (channels[2] - 0.406f) / 0.225f;

        cv::Mat normalized;
        cv::merge(channels, normalized);

        std::vector<cv::Mat> chw_channels;
        cv::split(normalized, chw_channels);

        cv::Mat blob(1, 3 * resize_h * resize_w, CV_32F);
        for (int c = 0; c < 3; c++) {
            memcpy(blob.ptr<float>(0) + c * resize_h * resize_w,
                chw_channels[c].data, resize_h * resize_w * sizeof(float));
        }

        return blob.reshape(1, { 1, 3, resize_h, resize_w });
    }

    cv::Mat RecPreprocess(const cv::Mat& image, int target_height, int& dst_width) {
        int h = image.rows;
        int w = image.cols;

        float ratio = static_cast<float>(w) / h;
        dst_width = static_cast<int>(target_height * ratio);
        dst_width = (dst_width + 3) / 4 * 4;
        dst_width = std::max(dst_width, 4);

        cv::Mat resized;
        cv::resize(image, resized, cv::Size(dst_width, target_height));

        cv::Mat float_img;
        resized.convertTo(float_img, CV_32F, 1.0 / 255.0);

        std::vector<cv::Mat> channels(3);
        cv::split(float_img, channels);

        for (int c = 0; c < 3; c++) {
            channels[c] = (channels[c] - 0.5f) / 0.5f;
        }

        cv::Mat normalized;
        cv::merge(channels, normalized);

        std::vector<cv::Mat> chw_channels;
        cv::split(normalized, chw_channels);

        cv::Mat blob(1, 3 * target_height * dst_width, CV_32F);
        for (int c = 0; c < 3; c++) {
            memcpy(blob.ptr<float>(0) + c * target_height * dst_width,
                chw_channels[c].data, target_height * dst_width * sizeof(float));
        }

        return blob.reshape(1, { 1, 3, target_height, dst_width });
    }

    // ========== Postprocess ==========

    std::vector<TextBox> DBPostprocess(const cv::Mat& pred, float thresh,
        float box_thresh, float unclip_ratio,
        const cv::Size& ori_size, float scale) {
        cv::Mat binary;
        cv::threshold(pred, binary, thresh * 255, 255, cv::THRESH_BINARY);
        binary.convertTo(binary, CV_8U);

        std::vector<std::vector<cv::Point>> contours;
        std::vector<cv::Vec4i> hierarchy;
        cv::findContours(binary, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

        std::vector<TextBox> boxes;
        for (auto& contour : contours) {
            float area = cv::contourArea(contour);
            if (area < 10) continue;

            cv::RotatedRect min_rect = cv::minAreaRect(contour);
            float rect_area = min_rect.size.width * min_rect.size.height;
            if (rect_area < 10) continue;

            cv::Mat box_mask = cv::Mat::zeros(pred.size(), CV_8U);
            std::vector<std::vector<cv::Point>> tmp = { contour };
            cv::drawContours(box_mask, tmp, 0, cv::Scalar(255), -1);

            cv::Mat box_pred;
            pred.copyTo(box_pred, box_mask);
            float score = static_cast<float>(cv::sum(box_pred)[0]) / (area + 1e-6f);

            if (score < box_thresh) continue;

            cv::Point2f vertices[4];
            min_rect.points(vertices);

            std::vector<cv::Point> box;
            for (int i = 0; i < 4; i++) {
                box.push_back(cv::Point(static_cast<int>(vertices[i].x / scale),
                    static_cast<int>(vertices[i].y / scale)));
            }

            box = UnclipPolygon(box, unclip_ratio);

            for (auto& pt : box) {
                pt.x = std::max(0, std::min(pt.x, ori_size.width - 1));
                pt.y = std::max(0, std::min(pt.y, ori_size.height - 1));
            }

            boxes.push_back({ box, score });
        }

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
                if (PolygonIoU(boxes[i].points, boxes[j].points) > 0.5f) {
                    suppressed[j] = true;
                }
            }
        }

        return result;
    }

    std::vector<cv::Point> UnclipPolygon(const std::vector<cv::Point>& box, float ratio) {
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
                static_cast<int>(center.x + vec.x * ratio),
                static_cast<int>(center.y + vec.y * ratio)
            ));
        }

        return expanded;
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

    // ========== 几何工具 ==========

    std::vector<cv::Point> OrderPointsClockwise(const std::vector<cv::Point>& pts) {
        std::vector<cv::Point> ordered(4);
        std::vector<cv::Point> tmp = pts;

        std::sort(tmp.begin(), tmp.end(), [](const cv::Point& a, const cv::Point& b) {
            return a.y < b.y;
            });

        if (tmp[0].x < tmp[1].x) {
            ordered[0] = tmp[0];
            ordered[1] = tmp[1];
        }
        else {
            ordered[0] = tmp[1];
            ordered[1] = tmp[0];
        }

        if (tmp[2].x < tmp[3].x) {
            ordered[3] = tmp[2];
            ordered[2] = tmp[3];
        }
        else {
            ordered[3] = tmp[3];
            ordered[2] = tmp[2];
        }

        return ordered;
    }

    cv::Mat CropAndWarpBox(const cv::Mat& image, const std::vector<cv::Point>& box) {
        auto ordered = OrderPointsClockwise(box);

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

    float PolygonIoU(const std::vector<cv::Point>& poly1,
        const std::vector<cv::Point>& poly2) {
        std::vector<cv::Point> inter;
        float area1 = cv::contourArea(poly1);
        float area2 = cv::contourArea(poly2);

        if (area1 < 1e-6f || area2 < 1e-6f) return 0.0f;

        cv::intersectConvexConvex(poly1, poly2, inter, false);
        float inter_area = cv::contourArea(inter);

        return inter_area / (area1 + area2 - inter_area + 1e-6f);
    }

} // namespace ocr_utils