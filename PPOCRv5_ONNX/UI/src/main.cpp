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

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cout << "Usage: " << argv[0]
            << " <det_onnx> <rec_onnx> <dict_txt> [image_path]" << std::endl;
        return -1;
    }

    std::string det_path = argv[1];
    std::string rec_path = argv[2];
    std::string dict_path = argv[3];
    std::string image_path = (argc > 4) ? argv[4] : "test.jpg";

    PPOCRv5System ocr;
    if (!ocr.Init(det_path, rec_path, dict_path, false)) {
        std::cerr << "Init failed!" << std::endl;
        return -1;
    }

    cv::Mat image = cv::imread(image_path);
    if (image.empty()) {
        std::cerr << "Failed to load image: " << image_path << std::endl;
        return -1;
    }

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

    cv::imwrite("result.jpg", image);
    std::cout << "Result saved to result.jpg" << std::endl;

    return 0;
}