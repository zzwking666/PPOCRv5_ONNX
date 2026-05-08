#pragma once

#include"IModule.hpp"
#include<QObject>
#include "oso/oso_StorageContext.hpp"
#include "BagsRealTimeDisplayInfo.hpp"
#include "SetConfig.hpp"

class ConfigModule
	: public QObject, public IModule<bool>
{
	Q_OBJECT
public:
	bool build() override;
	void destroy() override;
	void start() override;
	void stop() override;
public:
	std::unique_ptr<rw::oso::StorageContext> storeContext{ nullptr };
public:
	cdm::BagsRealTimeDisplayInfo bagsRealTimeDisplayInfo{};
	cdm::SetConfig setConfig{};
};
