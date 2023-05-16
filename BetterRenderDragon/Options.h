#pragma once
#include "Core/Math/Vec4.h"

class Options {
public:
	static bool showImGui;
	static bool deferredRenderingEnabled;
	static bool limitShaderModel;
	static bool disableRendererContextD3D12RTX;
	static bool redirectShaders;

	bool load();
	bool save();
};
