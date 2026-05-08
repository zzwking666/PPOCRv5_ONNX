#include "ImageStitch.hpp"
#include <algorithm>
#include <vector>
#include <cmath>
#include <QDebug>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include "imgqt/imgqt.hpp"

#include "Modules.hpp"


ImageStitch::ImageStitch(ConfigModule& configModule, QObject* parent)
	: QThread(parent), configModule(configModule)
{

}

ImageStitch::~ImageStitch()
{

}

void ImageStitch::onFrameCaptured(rw::rqwc::MatInfo matInfo, size_t index)
{
	if (matInfo.mat.empty())
	{
		return;
	}

	auto& statisticalInfo = Modules::getInstance().asynchronousThreadModule.statisticalInfo;

	if (2 == index)
	{
		if (configModule.setConfig.isjingxiang)
		{
			rotateXImage(matInfo.mat);
		}
		QImage qimg = rw::img::cvMatToQImage(matInfo.mat);
		emit imageReady(index, qimg);
		++statisticalInfo.beimianzongliang;
		const int frameNum = static_cast<int>(matInfo.frameInfo.frameNum);
		if(frameNum - lastCam2FrameNum > 1)
		{
			++statisticalInfo.cam2FrameLost;
		}
		lastCam2FrameNum = frameNum;
	}
	else
	{
		QImage qimg = rw::img::cvMatToQImage(matInfo.mat);
		emit imageReady(index, qimg);
		++statisticalInfo.zhengmianzongliang;
		const int frameNum = static_cast<int>(matInfo.frameInfo.frameNum);
		if (frameNum - lastCam1FrameNum > 1)
		{
			++statisticalInfo.cam1FrameLost;
		}
		lastCam1FrameNum = frameNum;
	}

	// ---------------------拼接图像的逻辑----------------------
	/*if (matInfos.size() > matInfosSize)
	{
		matInfos.pop_front();
	}
	matInfos.push_back(matInfo);*/

	// 更新最新的图像的帧号
	//newFrameNum = static_cast<int>(matInfo.frameInfo.frameNum);

	//auto& setConfig = Modules::getInstance().configModule.setConfig;
	//auto stitchNum = setConfig.qiehuanzhangshu;

	//auto stitchedMat = stitchImages(stitchNum);

	//  ----------------------拼接图像的逻辑----------------------

	// ----------------------4090测试用---------------------------------
	/*auto testImg1 = cv::imread("C:/Users/zfkj4090/Downloads/test_ocr.png");
	auto testImg2 = cv::imread("C:/Users/zfkj4090/Downloads/222.jpg");
	static bool emitFirst = true;
	if (testImg1.empty() || testImg2.empty())
	{
		return;
	}

	if (emitFirst)
	{
		const QImage qimg = rw::img::cvMatToQImage(testImg1);
		emit imageReady(1, qimg);
	}
	else
	{
		if (configModule.setConfig.isjingxiang)
		{
			rotateXImage(testImg2);
		}
		const QImage qimg = rw::img::cvMatToQImage(testImg2);
		emit imageReady(2, qimg);
	}

	emitFirst = !emitFirst;*/

	// ----------------------4090测试用---------------------------------
}

cv::Mat ImageStitch::stitchImages(int stitchCount)
{
	if (stitchCount <= 0 || matInfos.isEmpty())
	{
		return {};
	}

	const int latestFrame = newFrameNum;
	if (latestFrame <= 0)
	{
		return {};
	}

	const int minFrame = std::max(1, latestFrame - stitchCount + 1);

	QVector<rw::rqwc::MatInfo> selected;
	selected.reserve(stitchCount);

	for (int i = matInfos.size() - 1; i >= 0; --i)
	{
		const auto& info = matInfos[i];
		const int frameNum = static_cast<int>(info.frameInfo.frameNum);

		if (frameNum > latestFrame)
		{
			continue;
		}
		if (frameNum < minFrame)
		{
			break;
		}
		if (info.mat.empty())
		{
			continue;
		}

		selected.push_back(info);
		if (selected.size() == stitchCount)
		{
			break;
		}
	}

	if (selected.size() < stitchCount)
	{
		return {};
	}

	std::sort(selected.begin(), selected.end(),
		[](const rw::rqwc::MatInfo& a, const rw::rqwc::MatInfo& b)
		{
			return a.frameInfo.frameNum < b.frameInfo.frameNum;
		});

	for (const auto& info : selected)
	{
		if (static_cast<int>(info.frameInfo.frameNum) == lastStitchFrameNum)
		{
			return {};
		}
	}

	const int targetCols = selected.front().mat.cols;
	const int targetType = selected.front().mat.type();

	std::vector<cv::Mat> mats;
	mats.reserve(static_cast<size_t>(selected.size()));

	for (const auto& info : selected)
	{
		cv::Mat m = info.mat;

		if (m.type() != targetType)
		{
			cv::Mat converted;
			m.convertTo(converted, targetType);
			m = std::move(converted);
		}

		if (m.cols != targetCols)
		{
			const int newH = std::max(1, static_cast<int>(
				std::llround(static_cast<double>(m.rows) * targetCols / std::max(1, m.cols))));
			cv::resize(m, m, cv::Size(targetCols, newH), 0, 0, cv::INTER_LINEAR);
		}

		mats.push_back(std::move(m));
	}

	cv::Mat stitched;
	cv::vconcat(mats, stitched);

	lastStitchFrameNum = latestFrame;
	return stitched;
}

void ImageStitch::rotateXImage(cv::Mat& image)
{
	if (image.empty())
	{
		return;
	}

	// 以竖直中轴 x = (w - 1) / 2 做轴对称：x' = -x + 2*x0
	const double x0 = (static_cast<double>(image.cols) - 1.0) * 0.5;
	const cv::Mat M = (cv::Mat_<double>(2, 3) << -1.0, 0.0, 2.0 * x0,
		0.0, 1.0, 0.0);

	cv::Mat dst;
	cv::warpAffine(
		image, dst, M, image.size(),
		cv::INTER_LINEAR,
		cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0)
	);

	image = std::move(dst);
}
