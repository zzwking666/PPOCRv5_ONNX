#pragma once

#include "IModule.hpp"
#include "oso/oso_func.hpp"
#include "RunEnvCheck.hpp"
#include "CameraModule.hpp"
#include "ConfigModule.hpp"
#include "ImageStitch.hpp"
#include "AsynchronousThreadModule.hpp"

class Modules
	:public IModule<bool>
{

#pragma region Construction
public:
	static Modules& getInstance()
	{
		static Modules instance;
		return instance;
	}

	Modules(const Modules&);
	Modules& operator=(const Modules&) = delete;

private:
	Modules();
	~Modules() override;

public:
	bool build() override;
	void destroy() override;
	void start() override;
	void stop() override;
	void connect();
#pragma endregion
public:
	static bool check();
	template<class TypeCanToAssembly>
	static void checkFileExistAndFormat(const QString& path, const rw::oso::StorageContext& context);
	static bool EnsureDirectoryExists(const QString& dirPath);

public:
	CameraModule cameraModule;
	ConfigModule configModule;
	ImageStitch imageStitchModule;
	AsynchronousThreadModule asynchronousThreadModule;
};

template <class TypeCanToAssembly>
void Modules::checkFileExistAndFormat(const QString& path, const rw::oso::StorageContext& context)
{
	if (RunEnvCheck::isFileExist(path))
	{
		if (!RunEnvCheck::isFileFormatCorrectWithSafe<TypeCanToAssembly>(path, context))
		{
			auto assembly = context.load(path.toStdString());
			bool isMerge{ false };
			auto mergeAssembly = rw::oso::AssemblyMergeTool::Merge(TypeCanToAssembly(), *assembly, isMerge);
			if (isMerge)
			{
				context.saveSafe(mergeAssembly, path.toStdString());
			}
			else
			{
				context.saveSafe(TypeCanToAssembly(), path.toStdString());
			}
		}
	}
	else
	{
		context.saveSafe(TypeCanToAssembly(), path.toStdString());
	}
}

