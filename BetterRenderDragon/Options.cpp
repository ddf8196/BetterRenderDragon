#include <string>
#include <fstream>

#include <windows.storage.h>
#include <wrl.h>
#pragma comment(lib, "runtimeobject.lib")

#include "Options.h"

bool Options::showImGui = true;
bool Options::deferredRenderingEnabled = false;
bool Options::limitShaderModel = true;
bool Options::disableRendererContextD3D12RTX = false;
bool Options::redirectShaders = true;
