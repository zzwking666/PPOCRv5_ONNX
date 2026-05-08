#include "AsynchronousThreadModule.hpp"

bool AsynchronousThreadModule::build()
{
	refreshUIThread = std::make_unique<RefreshUIThread>(this);

	return true;
}

void AsynchronousThreadModule::destroy()
{
	if (refreshUIThread)
	{
		refreshUIThread.reset();
	}
}

void AsynchronousThreadModule::start()
{
	if (refreshUIThread)
	{
		refreshUIThread->startThread();
	}
}

void AsynchronousThreadModule::stop()
{
	if (refreshUIThread)
	{
		refreshUIThread->stopThread();
	}
}
