#pragma once

#include <QObject>
#include "Modules.hpp"
#include "BagsRealTimeDisplay.h"

class AppRuntime : public QObject
{
	Q_OBJECT
public:
	bool initialize();
	void show();
	void shutdown();

public:
	void build_connect();
public:
	Modules& _modules{ Modules::getInstance() };
	std::unique_ptr<BagsRealTimeDisplay> _bagsRealTimeDisplay{ nullptr };
};