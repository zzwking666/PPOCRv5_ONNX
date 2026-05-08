#pragma once

#include <QObject>
#include <QThread>

#include "rqwcd/rqwc_d.hpp"
#include "ConfigModule.hpp"

class ImageStitch : public QThread
{
	Q_OBJECT

public:
	explicit ImageStitch(ConfigModule& configModule,QObject *parent = nullptr);
	~ImageStitch() override;
public slots:
	// 相机回调函数
	void onFrameCaptured(rw::rqwc::MatInfo matInfo, size_t index);

private:
	// 拼接图像函数
	cv::Mat stitchImages(int stitchCount);

	void rotateXImage(cv::Mat& image);
private:
	QVector<rw::rqwc::MatInfo> matInfos{};
	int matInfosSize{ 30 };
	int lastStitchFrameNum{ 0 };
	int newFrameNum{ 0 };

	// 计算丢帧用的变量
	int lastCam1FrameNum{ 0 };
	int lastCam2FrameNum{ 0 };

	ConfigModule& configModule;
signals:
	void imageReady(size_t index, const QImage& image);
};
