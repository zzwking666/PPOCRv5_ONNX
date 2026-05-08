#pragma once

#include<QString>

#include "oso/oso_StorageContext.hpp"

class RunEnvCheck {
public:
	static bool isSingleInstance(const QString& instanceName);

	static bool isProcessRunning(const QString& processName);

	static bool isFileExist(const QString& filePath);

	template<class T>
	static bool isFileFormatCorrectWithSafe(const QString& filePath, const rw::oso::StorageContext& context);

	template<class T>
	static bool isFileFormatCorrect(const QString& filePath, const rw::oso::StorageContext& context);
};

template <class T>
bool RunEnvCheck::isFileFormatCorrectWithSafe(const QString& filePath, const rw::oso::StorageContext& context)
{
	if (!isFileExist(filePath)) {
		return false;
	}
	bool isGet{ false };
	T data = context.loadSafeToType<T>(filePath.toStdString(), isGet);

	return isGet;
}

template <class T>
bool RunEnvCheck::isFileFormatCorrect(const QString& filePath, const rw::oso::StorageContext& context)
{
	if (!isFileExist(filePath)) {
		return false;
	}
	bool isGet{ false };
	T data = context.loadToType<T>(filePath.toStdString(), isGet);
	return isGet;
}
