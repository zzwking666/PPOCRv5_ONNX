#include "DetModel.hpp"
#include "ocr_utils.hpp"
#include <iostream>
#include <filesystem>

DetModel::DetModel() : env_(ORT_LOGGING_LEVEL_WARNING, "det"),
memory_info_(Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault)) {}

DetModel::~DetModel() = default;

bool DetModel::Init(const std::string& model_path, const Config& cfg, bool use_gpu) {
    try {
        cfg_ = cfg;
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
        for (size_t i = 0; i < session_->GetInputCount(); i++) {
            input_names_.push_back(session_->GetInputNameAllocated(i, allocator).get());
        }
        for (size_t i = 0; i < session_->GetOutputCount(); i++) {
            output_names_.push_back(session_->GetOutputNameAllocated(i, allocator).get());
        }

        return true;
    }
    catch (const Ort::Exception& e) {
        std::cerr << "DetModel Init failed: " << e.what() << std::endl;
        return false;
    }
}

cv::Mat DetModel::InferRaw(const cv::Mat& image, float& scale) {
    if (!session_) return {};

    cv::Mat input_blob = ocr_utils::DetPreprocess(image, cfg_.target_size, scale);

    int h = input_blob.size[2];
    int w = input_blob.size[3];
    std::vector<int64_t> input_shape = { 1, 3, h, w };

    std::vector<float> input_data(input_blob.begin<float>(), input_blob.end<float>());

    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
        memory_info_, input_data.data(), input_data.size(), input_shape.data(), input_shape.size());

    auto output_tensors = session_->Run(
        Ort::RunOptions{ nullptr },
        input_names_.data(), &input_tensor, 1,
        output_names_.data(), output_names_.size());

    auto output_shape = output_tensors[0].GetTensorTypeAndShapeInfo().GetShape();
    float* output_data = output_tensors[0].GetTensorMutableData<float>();

    return cv::Mat(output_shape[2], output_shape[3], CV_32F, output_data);
}

std::vector<ocr_utils::TextBox> DetModel::Infer(const cv::Mat& image) {
    float scale = 1.0f;
    cv::Mat pred = InferRaw(image, scale);
    return ocr_utils::DBPostprocess(pred, cfg_.db_thresh, cfg_.db_box_thresh,
        cfg_.db_unclip_ratio, image.size(), scale);
}