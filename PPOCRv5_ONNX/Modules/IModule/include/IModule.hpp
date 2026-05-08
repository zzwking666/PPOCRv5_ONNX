#pragma once

#include<vector>
#include<qstring.h>

template<class TBuildError = bool>
class IModule
{
public:
	virtual ~IModule() = default;
	virtual TBuildError build() = 0;
	virtual void destroy() = 0;
public:
	virtual void start() = 0;
	virtual void stop() = 0;

};