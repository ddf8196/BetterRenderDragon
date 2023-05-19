#pragma once
#include <string>
#include <atomic>

class Options {
public:
	static bool showImGui;

	static bool performanceEnabled;

	static bool vanilla2DeferredEnabled;
	static bool deferredRenderingEnabled;
	static bool limitShaderModel;
	static bool disableRendererContextD3D12RTX;

	static bool materialBinLoaderEnabled;
	static bool redirectShaders;

	static bool customUniformsEnabled;

	static std::atomic_bool dirty;

	static bool init();
	static bool load();
	static bool save();

private:
	static std::string optionsDir;
	static std::string optionsFile;
};
