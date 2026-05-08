#include"CameraModule.hpp"
#include"Utility.hpp"
#include <QRegularExpression>

#include "Modules.hpp"

CameraModule::CameraModule()
{

}

CameraModule::~CameraModule()
{

}

bool CameraModule::build()
{
	if (!build_camera1())
	{
		qWarning("Camera 1 build failed");
	}
	if (!build_camera2())
	{
		qWarning("Camera 2 build failed");
	}
	return true;
}

void CameraModule::destroy()
{
	destroy_camera1();
	destroy_camera2();
}

void CameraModule::start()
{
	if (camera1)
	{
		camera1->startMonitor();
		onCameraStateChanged(1, true);
	}
	else
	{
		onCameraStateChanged(1, false);
	}

	if (camera2)
	{
		camera2->startMonitor();
		onCameraStateChanged(2, true);
	}
	else
	{
		onCameraStateChanged(2, false);
	}
}

void CameraModule::stop()
{
	if (camera1)
	{
		camera1->stopMonitor();
		camera1->disconnectCamera();
	}
	if (camera2)
	{
		camera2->stopMonitor();
		camera2->disconnectCamera();
	}
}

bool CameraModule::build_camera1()
{
	auto cameraList = rw::hoec::UtilityFunc<rw::hoec::CameraProvider::DVP>::getCameraInfoList();
	auto cameraMetaData1 = cameraMetaDataCheck(utility.cameraIp1, cameraList);

	if (cameraMetaData1.ip != "0")
	{
		try
		{
			camera1 = std::make_unique<rw::rqwc::DVPCameraPassive>();
			camera1->setIP(cameraMetaData1.ip);
			camera1->connectCamera();
			camera1->registerCallBackFunc();
			//camera1->startMonitor();
			setCamera1TriggerOff();

			auto cfg = Modules::getInstance().configModule.setConfig;

			camera1->setExposureTime(static_cast<size_t>(cfg.baoguang1));
			camera1->setGain(static_cast<size_t>(cfg.zengyi1));

			QObject::connect(camera1.get(), &rw::rqwc::DVPCameraPassive::callBackFuncPost,
				this, &CameraModule::onCamera1Capture);

			onCameraStateChanged(1, true);
			return true;
		}
		catch (const std::exception&)
		{
			onCameraStateChanged(1, false);
			return false;
		}
	}
	onCameraStateChanged(1, false);
	return false;
}

bool CameraModule::build_camera2()
{
	auto cameraList = rw::hoec::UtilityFunc<rw::hoec::CameraProvider::DVP>::getCameraInfoList();
	auto cameraMetaData2 = cameraMetaDataCheck(utility.cameraIp2, cameraList);

	if (cameraMetaData2.ip != "0")
	{
		try
		{
			camera2 = std::make_unique<rw::rqwc::DVPCameraPassive>();
			camera2->setIP(cameraMetaData2.ip);
			camera2->connectCamera();
			camera2->registerCallBackFunc();
			//camera2->startMonitor();
			setCamera2TriggerOff();

			auto cfg = Modules::getInstance().configModule.setConfig;

			camera2->setExposureTime(static_cast<size_t>(cfg.baoguang2));
			camera2->setGain(static_cast<size_t>(cfg.zengyi2));

			QObject::connect(camera2.get(), &rw::rqwc::DVPCameraPassive::callBackFuncPost,
				this, &CameraModule::onCamera2Capture);

			onCameraStateChanged(2, true);
			return true;
		}
		catch (const std::exception&)
		{
			onCameraStateChanged(2, false);
			return false;
		}
	}
	onCameraStateChanged(2, false);
	return false;
}

void CameraModule::destroy_camera1()
{
	if (camera1)
	{
		camera1.reset();
		onCameraStateChanged(1, false);
	}
}

void CameraModule::destroy_camera2()
{
	if (camera2)
	{
		camera2.reset();
		onCameraStateChanged(2, false);
	}
}

void CameraModule::setCamera1TriggerOff()
{
	if (camera1)
	{
		camera1->setTriggerModeStatus(rw::hoec::TriggerModeStatus::OFF);
		isCamera1SoftTrigger = true;
	}
}

void CameraModule::setCamera1HardwareTrigger()
{
	if (camera1)
	{
		camera1->setTriggerModeStatus(rw::hoec::TriggerModeStatus::ON);
		camera1->setTriggerSource(rw::hoec::TriggerSource::Line0);
		isCamera1SoftTrigger = false;
	}
}

void CameraModule::setCamera2TriggerOff()
{
	if (camera2)
	{
		camera2->setTriggerModeStatus(rw::hoec::TriggerModeStatus::OFF);
		isCamera2SoftTrigger = true;
	}
}

void CameraModule::setCamera2HardwareTrigger()
{
	if (camera2)
	{
		camera2->setTriggerModeStatus(rw::hoec::TriggerModeStatus::ON);
		camera2->setTriggerSource(rw::hoec::TriggerSource::Line0);
		isCamera2SoftTrigger = false;
	}
}

void CameraModule::setCamera1ExposureTime(size_t exposureTime)
{
	if (camera1)
	{
		camera1->setExposureTime(exposureTime);
	}
}

void CameraModule::setCamera1Gain(size_t gain)
{
	if (camera1)
	{
		camera1->setGain(gain);
	}
}

void CameraModule::setCamera2ExposureTime(size_t exposureTime)
{
	if (camera2)
	{
		camera2->setExposureTime(exposureTime);
	}
}

void CameraModule::setCamera2Gain(size_t gain)
{
	if (camera2)
	{
		camera2->setGain(gain);
	}
}

void CameraModule::setCamera1Multiplier(size_t multiplier)
{
	if (camera1)
	{
		camera1->setMultiplier(multiplier);
	}
}

void CameraModule::setCamera1PostDivider(size_t postDivider)
{
	if (camera1)
	{
		camera1->setPostDivider(postDivider);
	}
}

void CameraModule::setCamera2Multiplier(size_t multiplier)
{
	if (camera2)
	{
		camera2->setMultiplier(multiplier);
	}
}

void CameraModule::setCamera2PostDivider(size_t postDivider)
{
	if (camera2)
	{
		camera2->setPostDivider(postDivider);
	}
}

bool CameraModule::isTargetCamera(const QString& cameraIndex, const QString& targetName)
{
	QRegularExpression regex(R"((\d+)\.(\d+)\.(\d+)\.(\d+))");
	QRegularExpressionMatch match = regex.match(targetName);

	if (match.hasMatch()) {
		auto matchString = match.captured(3);

		return cameraIndex == matchString;
	}

	return false;
}

rw::hoec::CameraInfo CameraModule::cameraMetaDataCheck(const QString& cameraIndex,
	const std::vector<rw::hoec::CameraInfo>& cameraInfo)
{
	for (const auto& cameraMetaData : cameraInfo) {
		if (isTargetCamera(cameraIndex, QString::fromStdString(cameraMetaData.ip))) {
			return cameraMetaData;
		}
	}
	rw::hoec::CameraInfo error;
	error.ip = "0";
	return error;
}

bool CameraModule::onBuildCamera(int index)
{
	switch (index)
	{
	case 1:
		return build_camera1();
	case 2:
		return build_camera2();
	default:
		return false;
	}
}

void CameraModule::onDestroyCamera(int index)
{
	switch (index)
	{
	case 1:
		destroy_camera1();
		break;
	case 2:
		destroy_camera2();
		break;
	default:
		break;
	}
}

void CameraModule::onStartCamera(int index)
{
	switch (index)
	{
	case 1:
		if (camera1)
		{
			camera1->startMonitor();
		}
		break;
	case 2:
		if (camera2)
		{
			camera2->startMonitor();
		}
		break;
	default:
		break;
	}
}

void CameraModule::onCamera1Capture(const rw::hoec::MatInfo& matInfo)
{
	emit onCameraCapture(matInfo, 1);
}

void CameraModule::onCamera2Capture(const rw::hoec::MatInfo& matInfo)
{
	emit onCameraCapture(matInfo, 2);
}
