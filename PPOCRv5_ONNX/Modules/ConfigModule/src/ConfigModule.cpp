#include"ConfigModule.hpp"

#include <oso/oso_StorageContext.hpp>

#include "utility.hpp"

bool ConfigModule::build()
{
	storeContext = std::make_unique<rw::oso::StorageContext>(rw::oso::StorageType::Xml);

	auto loadBagsRealTimeDisplayInfo = storeContext->loadSafe(globalPath.bagsRealTimeDisplayConfigPath.toStdString());

	if (loadBagsRealTimeDisplayInfo)
	{
		bagsRealTimeDisplayInfo = *loadBagsRealTimeDisplayInfo;
	}
	else
	{
		qWarning("主窗体参数加载失败!");
	}

	auto loadSetConfig = storeContext->loadSafe(globalPath.setConfigPath.toStdString());

	if (loadSetConfig)
	{
		setConfig = *loadSetConfig;
	}
	else
	{
		qWarning("设置参数加载失败!");
	}

	return true;
}

void ConfigModule::destroy()
{
	storeContext->saveSafe(bagsRealTimeDisplayInfo, globalPath.bagsRealTimeDisplayConfigPath.toStdString());
	storeContext->saveSafe(setConfig, globalPath.setConfigPath.toStdString());
	storeContext.reset();
}

void ConfigModule::start()
{

}

void ConfigModule::stop()
{

}
