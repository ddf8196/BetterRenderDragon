#pragma once
#include <string>
#include <atomic>

namespace Options {
	extern bool showImGui;

	extern bool performanceEnabled;

	extern bool windowSettingsEnabled;

	extern bool vanilla2DeferredAvailable;
	extern bool vanilla2DeferredEnabled;
	extern bool deferredRenderingEnabled;
	extern bool newVideoSettingsAvailable;
	extern bool forceEnableDeferredTechnicalPreview;
	extern bool disableRendererContextD3D12RTX;

	extern bool materialBinLoaderEnabled;
	extern bool redirectShaders;
	extern bool reloadShadersAvailable;
	extern std::atomic_bool reloadShaders;

	extern bool customUniformsEnabled;

	extern int uiKey;
	extern int reloadShadersKey;

	extern std::atomic_bool dirty;

	extern std::string optionsDir;
	extern std::string optionsFile;

	extern bool init();
	extern bool load();
	extern bool save();
};
