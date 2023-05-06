#pragma once
#include <cstdint>

namespace dragon {
	namespace materials {
		enum class ShaderCodePlatform : uint8_t {
			Direct3D_SM40,
			Direct3D_SM50,
			Direct3D_SM60,
			Direct3D_SM65,
			Direct3D_XB1,
			Direct3D_XBX,
			GLSL_120,
			GLSL_430,
			ESSL_100,
			ESSL_300,
			ESSL_310,
			Metal,
			Vulkan,
			Nvn,
			Pssl,
			Unknown
		};
	}
}
