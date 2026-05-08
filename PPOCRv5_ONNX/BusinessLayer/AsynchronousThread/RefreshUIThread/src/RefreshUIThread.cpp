#include "RefreshUIThread.hpp"

RefreshUIThread::RefreshUIThread(QObject* parent)
{
}

RefreshUIThread::~RefreshUIThread()
{
	stopThread();
	wait();
}

void RefreshUIThread::startThread()
{
	running = true;
	if (!isRunning()) {
		start();
	}
}

void RefreshUIThread::stopThread()
{
	running = false;
}

void RefreshUIThread::run()
{
	while (running)
	{
		QThread::msleep(1000);
		emit emit_RefreshUI();
	}
}
