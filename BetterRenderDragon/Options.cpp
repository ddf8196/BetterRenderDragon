#include <string>
#include <fstream>

#include <windows.storage.h>
#include <wrl.h>
#pragma comment(lib, "runtimeobject.lib")

#include "Options.h"

bool Options::showImGui = true;

bool Options::performanceEnabled = true;

bool Options::vanilla2DeferredEnabled = true;
bool Options::deferredRenderingEnabled = false;
bool Options::limitShaderModel = true;
bool Options::disableRendererContextD3D12RTX = false;

bool Options::materialBinLoaderEnabled = true;
bool Options::redirectShaders = true;

bool Options::customUniformsEnabled = false;

bool Options::load() {
	return false;
}

bool Options::save() {
	return false;
}