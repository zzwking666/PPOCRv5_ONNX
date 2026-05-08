#pragma once

#include "IModule.hpp"
#include <QObject>
#include "RefreshUIThread.hpp"

class AsynchronousThreadModule
	: public QObject, public IModule<bool>
{
	Q_OBJECT
public:
	bool build() override;
	void destroy() override;
	void start() override;
	void stop() override;
public:
	struct StatisticalInfo
	{
		std::atomic_uint64_t zhengmianzongliang{ 0 };
		std::atomic_uint64_t beimianzongliang{ 0 };

		std::atomic_uint64_t cam1FrameLost{ 0 };
		std::atomic_uint64_t cam2FrameLost{ 0 };
	} statisticalInfo;
public:
	std::unique_ptr<RefreshUIThread> refreshUIThread{ nullptr };
};
