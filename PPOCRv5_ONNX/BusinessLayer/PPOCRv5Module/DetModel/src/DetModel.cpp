#include "DetModel.hpp"
#include "ocr_utils.hpp"
#include <iostream>
#include <filesystem>
#include <algorithm>

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

        std::cout << "[DetModel] Init OK" << std::endl;
        std::cout << "  model_path: " << model_path << std::endl;
        std::cout << "  use_gpu: " << (use_gpu ? "true" : "false") << std::endl;
        std::cout << "  cfg: target_size=" << cfg_.target_size
                  << ", db_thresh=" << cfg_.db_thresh
                  << ", db_box_thresh=" << cfg_.db_box_thresh
                  << ", db_unclip_ratio=" << cfg_.db_unclip_ratio << std::endl;
        std::cout << "  input_count=" << input_names_.size() << ", output_count=" << output_names_.size() << std::endl;
        for (size_t i = 0; i < input_name_storage_.size(); ++i) {
            std::cout << "  input[" << i << "]: " << input_name_storage_[i] << std::endl;
        }
        for (size_t i = 0; i < output_name_storage_.size(); ++i) {
            std::cout << "  output[" << i << "]: " << output_name_storage_[i] << std::endl;
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

    std::cout << "[DetModel] InferRaw" << std::endl;
    std::cout << "  image: " << image.cols << "x" << image.rows << " channels=" << image.channels() << std::endl;
    std::cout << "  input_shape: [" << input_shape[0] << ", " << input_shape[1] << ", " << input_shape[2] << ", " << input_shape[3] << "]" << std::endl;
    std::cout << "  input_data.size=" << input_data.size() << " scale=" << scale << std::endl;

    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
        memory_info_, input_data.data(), input_data.size(), input_shape.data(), input_shape.size());

    auto output_tensors = session_->Run(
        Ort::RunOptions{ nullptr },
        input_names_.data(), &input_tensor, 1,
        output_names_.data(), output_names_.size());

    auto output_shape = output_tensors[0].GetTensorTypeAndShapeInfo().GetShape();
    float* output_data = output_tensors[0].GetTensorMutableData<float>();

    cv::Mat prob((int)output_shape[2], (int)output_shape[3], CV_32F, output_data);

    double min_v = 0.0;
    double max_v = 0.0;
    cv::minMaxLoc(prob, &min_v, &max_v);
    cv::Scalar mean_v = cv::mean(prob);

    std::cout << "  output_shape: [";
    for (size_t i = 0; i < output_shape.size(); ++i) {
        std::cout << output_shape[i] << (i + 1 == output_shape.size() ? "" : ", ");
    }
    std::cout << "]" << std::endl;
    std::cout << "  prob stats: min=" << min_v << ", max=" << max_v << ", mean=" << mean_v[0] << std::endl;

    return prob.clone();
}

std::vector<ocr_utils::TextBox> DetModel::Infer(const cv::Mat& image) {
    float scale = 1.0f;
    cv::Mat pred = InferRaw(image, scale);
    auto boxes = ocr_utils::DBPostprocess(pred, cfg_.db_thresh, cfg_.db_box_thresh,
        cfg_.db_unclip_ratio, image.size(), scale);

    std::cout << "[DetModel] DBPostprocess boxes=" << boxes.size()
              << " (db_thresh=" << cfg_.db_thresh
              << ", db_box_thresh=" << cfg_.db_box_thresh
              << ", unclip=" << cfg_.db_unclip_ratio << ")" << std::endl;

    return boxes;
}