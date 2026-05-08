#include "DetModel.hpp"
#include "Preprocess.hpp"
#include "Postprocess.hpp"
#include <iostream>

DetModel::DetModel() : env_(ORT_LOGGING_LEVEL_WARNING, "det"),
memory_info_(Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault)) {}

DetModel::~DetModel() = default;

bool DetModel::Init(const std::string& model_path, bool use_gpu) {
    try {
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
        std::cerr << "DetModel Init failed: " << e.what() << std::endl;
        return false;
    }
}

std::vector<TextBox> DetModel::Infer(const cv::Mat& image) {
    if (!session_) return {};

    float scale = 1.0f;
    cv::Mat input_blob = Preprocess(image, scale);

    // 获取动态输入尺寸
    int h = input_blob.size[2];
    int w = input_blob.size[3];
    std::vector<int64_t> input_shape = { 1, 3, h, w };

    // 准备输入数据
    std::vector<float> input_data(input_blob.begin<float>(), input_blob.end<float>());

    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
        memory_info_, input_data.data(), input_data.size(), input_shape.data(), input_shape.size());

    // 推理
    auto output_tensors = session_->Run(
        Ort::RunOptions{ nullptr },
        input_names_.data(), &input_tensor, 1,
        output_names_.data(), output_names_.size());

    // 获取输出
    auto output_info = output_tensors[0].GetTensorTypeAndShapeInfo();
    auto output_shape = output_info.GetShape();

    float* output_data = output_tensors[0].GetTensorMutableData<float>();
    cv::Mat pred(output_shape[2], output_shape[3], CV_32F, output_data);

    return Postprocess(pred, scale, image.size());
}

cv::Mat DetModel::Preprocess(const cv::Mat& image, float& scale) {
    return Preprocess::DetPreprocess(image, target_size_, scale);
}

std::vector<TextBox> DetModel::Postprocess(const cv::Mat& pred, float scale,
    const cv::Size& ori_size) {
    return Postprocess::DBPostprocess(pred, det_db_thresh_, det_db_box_thresh_,
        det_db_unclip_ratio_, ori_size, scale);
}