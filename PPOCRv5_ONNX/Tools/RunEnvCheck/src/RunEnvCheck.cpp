#include"RunEnvCheck.hpp"

#include<QProcess>
#include<QSharedMemory>
#include<QFile>

bool RunEnvCheck::isSingleInstance(const QString& instanceName)
{
	static QSharedMemory sharedMemory(instanceName);
	if (!sharedMemory.create(1)) {
		return false;
	}
	return true;
}

bool RunEnvCheck::isProcessRunning(const QString& processName)
{
	QProcess process;
	process.start("tasklist", QStringList() << "/FI" << QString("IMAGENAME eq %1").arg(processName));
	process.waitForFinished();
	QString output = process.readAllStandardOutput();
	return output.contains(processName, Qt::CaseInsensitive);
}

bool RunEnvCheck::isFileExist(const QString& filePath)
{
	QFile file(filePath);
	return file.exists();
}