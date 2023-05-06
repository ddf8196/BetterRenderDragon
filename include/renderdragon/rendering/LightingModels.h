#pragma once
#include <cstdint>

namespace dragon {
	namespace rendering {
		enum class LightingModels : int32_t {
			Vanilla = 0,
			Deferred = 1,
			RTX = 2
		};
	}
}
