#pragma once
#include <QString>

inline struct
{
	QString cameraIp1 = "1";
	QString cameraIp2 = "2";
}utility;

inline struct
{
public:
	QString projectHome = R"(D:\zfkjData\BagsRealTimeDisplay\)";
public:
	QString configRootPath = projectHome + R"(config\)";
	QString modelRootPath = projectHome + R"(model\)";
	QString bagsRealTimeDisplayConfigPath = configRootPath + R"(BagsRealTimeDisplay.xml)";
	QString setConfigPath = configRootPath + R"(setConfig.xml)";
public:
	QString companyRootPath = projectHome + R"(txt\)";
	QString companyTxtPath = companyRootPath + R"(company.txt)";
}globalPath;