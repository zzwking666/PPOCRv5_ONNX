//#include <QFile>
//
//#include <QtWidgets/QApplication>
//#include "rqwu/rqwu_core.h"
//#include "AppRuntime.hpp"
//int main(int argc, char *argv[])
//{
//    QApplication app(argc, argv);
//    Q_INIT_RESOURCE(BagsRealTimeDisplay);
//    rw::rqwu::ini();
//
//	AppRuntime appRuntime;
//    if (!appRuntime.initialize())
//    {
//        return 1;
//    }
//
//    appRuntime.show();
//
//    QObject::connect(&app, &QCoreApplication::aboutToQuit,
//        [&appRuntime]()
//        {
//	        appRuntime.shutdown();
//        }
//    );
//  
//    return app.exec();
//}

// ----------------------------------OCR----------------------------------

#include "PPOCRv5System.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    // ========== 硬编码路径 ==========
    std::string det_path = R"(C:\Users\zfkj4090\PP-OCRv5_server_det_onnx\inference.onnx)";
    std::string rec_path = R"(C:\Users\zfkj4090\PP-OCRv5_server_rec_onnx\inference.onnx)";
    std::string dict_path = R"(C:\Users\zfkj4090\ppocr_v5_dict.txt)";
    std::string image_path = R"(C:\Users\zfkj4090\Downloads\test_ocr.png)";  // 你的测试图片路径

    // 如果命令行传了参数，优先使用命令行参数（保留灵活性）
    if (argc > 1) det_path = argv[1];
    if (argc > 2) rec_path = argv[2];
    if (argc > 3) dict_path = argv[3];
    if (argc > 4) image_path = argv[4];

    // ========== 初始化 ==========
    PPOCRv5System ocr;
    if (!ocr.Init(det_path, rec_path, dict_path, false)) {
        std::cerr << "Init failed!" << std::endl;

        // 调试时暂停，方便看错误信息
        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.get();
        return -1;
    }

    // ========== 读取图像 ==========
    cv::Mat image = cv::imread(image_path);
    if (image.empty()) {
        std::cerr << "Failed to load image: " << image_path << std::endl;

        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.get();
        return -1;
    }

    // ========== 执行 OCR ==========
    auto results = ocr.OCR(image);

    std::cout << "Detected " << results.size() << " text regions:" << std::endl;
    for (size_t i = 0; i < results.size(); i++) {
        std::cout << "[" << i << "] Text: " << results[i].text
            << " | Score: " << results[i].score << std::endl;

        for (size_t j = 0; j < results[i].box.size(); j++) {
            cv::line(image, results[i].box[j],
                results[i].box[(j + 1) % 4], cv::Scalar(0, 255, 0), 2);
        }
    }

    // ========== 保存结果 ==========
    std::string output_path = R"(C:\Users\zfkj4090\i.png)";
    try {
        if (!cv::haveImageWriter(output_path)) {
            std::cerr << "No writer found for output path extension: " << output_path << std::endl;
            std::cout << "Press Enter to exit..." << std::endl;
            std::cin.get();
            return -1;
        }

        if (!cv::imwrite(output_path, image)) {
            std::cerr << "Failed to save image: " << output_path << std::endl;
            std::cout << "Press Enter to exit..." << std::endl;
            std::cin.get();
            return -1;
        }

        std::cout << "Result saved to " << output_path << std::endl;
    }
    catch (const cv::Exception& e) {
        std::cerr << "OpenCV exception while saving image: " << e.what() << std::endl;
        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.get();
        return -1;
    }

    // 调试时暂停
    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();

    return 0;
}