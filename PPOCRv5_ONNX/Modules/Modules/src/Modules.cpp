#include"Modules.hpp"
#include"qmessagebox.h"
#include"Utility.hpp"
#include"QDir"

Modules::Modules(const Modules&)
	: imageStitchModule(configModule)
{

}

Modules::Modules()
	: imageStitchModule(configModule)
{}

Modules::~Modules()
{}

bool Modules::build()
{
	auto isConfigBuildSuccess = configModule.build();

	if (!isConfigBuildSuccess)
	{
		qWarning("Config build failed");
	}

	auto isCameraBuildSuccess = cameraModule.build();

	if (!isCameraBuildSuccess)
	{
		qWarning("Camera build failed");
	}

	auto isAsynchronousThreadBuildSuccess = asynchronousThreadModule.build();

	if (!isAsynchronousThreadBuildSuccess)
	{
		qWarning("AsynchronousThread build failed");
	}

	return true;
}

void Modules::destroy()
{
	asynchronousThreadModule.destroy();
	cameraModule.destroy();
	configModule.destroy();
}

void Modules::start()
{
	configModule.start();
	cameraModule.start();
	asynchronousThreadModule.start();
}

void Modules::stop()
{
	asynchronousThreadModule.stop();
	cameraModule.stop();
	configModule.stop();
}

void Modules::connect()
{
	// 相机出图信号连接到图像拼接模块
	QObject::connect(&cameraModule, &CameraModule::onCameraCapture,
		&imageStitchModule, &ImageStitch::onFrameCaptured);
}

bool Modules::check()
{
#pragma region check single instance
	if (!RunEnvCheck::isSingleInstance("BagsRealTimeDisplay.exe"))
	{
		QMessageBox::warning(nullptr, "错误", "已经有程序在运行，请勿多次打开");
		return false;
	}
#pragma endregion

#pragma region check run env
	if (RunEnvCheck::isProcessRunning("MVS.exe"))
	{
		QMessageBox::warning(nullptr, "错误", "检测到海康威视软件正在运行，请先关闭后再启动本程序。");
		return false;
	}

	if (RunEnvCheck::isProcessRunning("BasedCam3.exe"))
	{
		QMessageBox::warning(nullptr, "错误", "检测到度申相机平台软件正在运行，请先关闭后再启动本程序。");
		return false;
	}
#pragma endregion

#pragma region check directory exist
	EnsureDirectoryExists(globalPath.projectHome);
	EnsureDirectoryExists(globalPath.configRootPath);
	EnsureDirectoryExists(globalPath.modelRootPath);
	EnsureDirectoryExists(globalPath.companyRootPath);
#pragma endregion
	return true;
}

bool Modules::EnsureDirectoryExists(const QString& dirPath)
{
	QDir dir(dirPath);
	if (!dir.exists()) {
		return dir.mkpath(".");
	}
	return true;
}
