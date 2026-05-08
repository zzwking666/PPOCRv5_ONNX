#include "RecModel.hpp"
#include "Preprocess.hpp"
#include "Postprocess.hpp"
#include <iostream>
#include <fstream>

RecModel::RecModel() : env_(ORT_LOGGING_LEVEL_WARNING, "rec"),
memory_info_(Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault)) {}

RecModel::~RecModel() = default;

bool RecModel::Init(const std::string& model_path, const std::string& dict_path,
    bool use_gpu) {
    try {
        // 加载字典
        std::ifstream fs(dict_path);
        if (!fs.is_open()) {
            std::cerr << "Failed to open dict: " << dict_path << std::endl;
            return false;
        }
        std::string line;
        while (std::getline(fs, line)) {
            if (!line.empty()) dict_.push_back(line);
        }
        blank_idx_ = static_cast<int>(dict_.size());  // CTC blank index
        dict_.push_back(" ");  // blank token

        session_options_ = Ort::SessionOptions();
        session_options_.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
        session_options_.SetIntraOpNumThreads(4);

        if (use_gpu) {
            OrtCUDAProviderOptions cuda_options;
            session_options_.AppendExecutionProvider_CUDA(cuda_options);
        }

        session_ = std::make_unique<Ort::Session>(env_, model_path.c_str(), session_options_);

        Ort::AllocatorWithDefaultOptions allocator;
        for (size_t i = 0; i < session_->GetInputCount(); i++) {
            input_names_.push_back(session_->GetInputNameAllocated(i, allocator).get());
        }
        for (size_t i = 0; i < session_->GetOutputCount(); i++) {
            output_names_.push_back(session_->GetOutputNameAllocated(i, allocator).get());
        }

        return true;
    }
    catch (const Ort::Exception& e) {
        std::cerr << "RecModel Init failed: " << e.what() << std::endl;
        return false;
    }
}

std::pair<std::string, float> RecModel::Infer(const cv::Mat& image) {
    if (!session_) return { "", 0.0f };

    int dst_width = 0;
    cv::Mat input_blob = Preprocess(image, dst_width);

    std::vector<int64_t> input_shape = { 1, 3, rec_image_height_, dst_width };
    std::vector<float> input_data(input_blob.begin<float>(), input_blob.end<float>());

    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
        memory_info_, input_data.data(), input_data.size(), input_shape.data(), input_shape.size());

    auto output_tensors = session_->Run(
        Ort::RunOptions{ nullptr },
        input_names_.data(), &input_tensor, 1,
        output_names_.data(), output_names_.size());

    auto output_info = output_tensors[0].GetTensorTypeAndShapeInfo();
    auto output_shape = output_info.GetShape();

    // output_shape: [batch, seq_len, num_classes]
    int seq_len = static_cast<int>(output_shape[1]);
    int num_classes = static_cast<int>(output_shape[2]);

    float* output_data = output_tensors[0].GetTensorMutableData<float>();
    std::vector<float> output_vec(output_data, output_data + seq_len * num_classes);

    float score = 0.0f;
    std::string text = Postprocess(output_vec, seq_len, score);

    return { text, score };
}

cv::Mat RecModel::Preprocess(const cv::Mat& image, int& dst_width) {
    return Preprocess::RecPreprocess(image, rec_image_height_, dst_width);
}

std::string RecModel::Postprocess(const std::vector<float>& output, int seq_len, float& score) {
    // 取每个时间步的最大值索引
    std::vector<int> preds;
    int num_classes = static_cast<int>(dict_.size());

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
    }

    return Postprocess::CTCDecode(preds, dict_, blank_idx_);
}