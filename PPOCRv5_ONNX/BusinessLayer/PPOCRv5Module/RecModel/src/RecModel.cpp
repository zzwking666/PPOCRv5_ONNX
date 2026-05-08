#include "RecModel.hpp"
#include "ocr_utils.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>

RecModel::RecModel() : env_(ORT_LOGGING_LEVEL_WARNING, "rec"),
memory_info_(Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault)) {}

RecModel::~RecModel() = default;

bool RecModel::Init(const std::string& model_path, const std::string& dict_path,
    const Config& cfg, bool use_gpu) {
    try {
        cfg_ = cfg;

        std::ifstream fs(dict_path);
        if (!fs.is_open()) {
            std::cerr << "Failed to open dict: " << dict_path << std::endl;
            return false;
        }
        std::string line;
        while (std::getline(fs, line)) {
            if (!line.empty()) dict_.push_back(line);
        }
        blank_idx_ = static_cast<int>(dict_.size());
        dict_.push_back(" ");

        session_options_ = Ort::SessionOptions();
        session_options_.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
        session_options_.SetIntraOpNumThreads(4);

        if (use_gpu) {
            OrtCUDAProviderOptions cuda_options;
            session_options_.AppendExecutionProvider_CUDA(cuda_options);
        }

#ifdef _WIN32
        const std::wstring model_path_w = std::filesystem::path(model_path).wstring();
        session_ = std::make_unique<Ort::Session>(env_, model_path_w.c_str(), session_options_);
#else
        session_ = std::make_unique<Ort::Session>(env_, model_path.c_str(), session_options_);
#endif

        Ort::AllocatorWithDefaultOptions allocator;
        input_name_storage_.clear();
        output_name_storage_.clear();
        input_names_.clear();
        output_names_.clear();
        input_name_storage_.reserve(session_->GetInputCount());
        output_name_storage_.reserve(session_->GetOutputCount());
        input_names_.reserve(session_->GetInputCount());
        output_names_.reserve(session_->GetOutputCount());

        for (size_t i = 0; i < session_->GetInputCount(); i++) {
            auto input_name = session_->GetInputNameAllocated(i, allocator);
            input_name_storage_.emplace_back(input_name.get());
        }
        for (size_t i = 0; i < session_->GetOutputCount(); i++) {
            auto output_name = session_->GetOutputNameAllocated(i, allocator);
            output_name_storage_.emplace_back(output_name.get());
        }

        for (auto& name : input_name_storage_) {
            input_names_.push_back(name.c_str());
        }
        for (auto& name : output_name_storage_) {
            output_names_.push_back(name.c_str());
        }

        return true;
    }
    catch (const Ort::Exception& e) {
        std::cerr << "RecModel Init failed: " << e.what() << std::endl;
        return false;
    }
}

std::vector<float> RecModel::InferRaw(const cv::Mat& image, int& seq_len) {
    if (!session_) return {};

    int dst_width = 0;
    cv::Mat input_blob = ocr_utils::RecPreprocess(image, cfg_.rec_image_height, dst_width);

    std::vector<int64_t> input_shape = { 1, 3, cfg_.rec_image_height, dst_width };
    std::vector<float> input_data(input_blob.begin<float>(), input_blob.end<float>());

    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
        memory_info_, input_data.data(), input_data.size(), input_shape.data(), input_shape.size());

    auto output_tensors = session_->Run(
        Ort::RunOptions{ nullptr },
        input_names_.data(), &input_tensor, 1,
        output_names_.data(), output_names_.size());

    auto output_shape = output_tensors[0].GetTensorTypeAndShapeInfo().GetShape();
    seq_len = static_cast<int>(output_shape[1]);
    int num_classes = static_cast<int>(output_shape[2]);

    float* output_data = output_tensors[0].GetTensorMutableData<float>();
    return std::vector<float>(output_data, output_data + seq_len * num_classes);
}

std::pair<std::string, float> RecModel::Infer(const cv::Mat& image) {
    int seq_len = 0;
    auto output = InferRaw(image, seq_len);

    int num_classes = static_cast<int>(dict_.size());
    std::vector<int> preds;
    float total_score = 0.0f;
    int valid_count = 0;

    for (int t = 0; t < seq_len; t++) {
        int max_idx = 0;
        float max_val = output[t * num_classes];
        for (int c = 1; c < num_classes; c++) {
            if (output[t * num_classes + c] > max_val) {
                max_val = output[t * num_classes + c];
                max_idx = c;
            }
        }
        preds.push_back(max_idx);

        if (max_idx != blank_idx_) {
            total_score += max_val;
            valid_count++;
        }
    }

    std::string text = ocr_utils::CTCDecode(preds, dict_, blank_idx_);
    float score = (valid_count > 0) ? (total_score / valid_count) : 0.0f;

    return { text, score };
}