#include "HookAPI.h"
#include "MCHooks.h"
#include "Options.h"

#include "renderdragon/materials/ShaderCodePlatform.h"
#include "renderdragon/rendering/LightingModels.h"

using dragon::rendering::LightingModels;
LightingModels(*RayTracingOptions_getLightingModel)(void* self) = nullptr;
LightingModels RayTracingOptions_getLightingModel_Hook(void* self) {
	LightingModels result = RayTracingOptions_getLightingModel(self);
	if (result == LightingModels::Vanilla && Options::deferredRenderingEnabled) {
		result = LightingModels::Deferred;
	}
	return result;
}

DeclareHook(RayTracingOptions_isRayTracingAvailable, bool, void* self) {
	printf("RayTracingOptions::isRayTracingAvailable\n");

	ReplaceVtable(*(void**)self, 8, (void**)&RayTracingOptions_getLightingModel, RayTracingOptions_getLightingModel_Hook);
	bool result = original(self);
	Unhook(RayTracingOptions_isRayTracingAvailable);
	return result;
}

using dragon::materials::ShaderCodePlatform;
DeclareHook(dragon_bgfximpl_getShaderCodePlatform, ShaderCodePlatform) {
	ShaderCodePlatform result = original();
	if (result == ShaderCodePlatform::Direct3D_SM65 && Options::limitShaderModel) {
		result = ShaderCodePlatform::Direct3D_SM60;
	}
	return result;
}

void MCHooks_Init() {
	printf("%s\n", __FUNCTION__);

	//RayTracingOptions::isRayTracingAvailable
	//1.19.80
	uintptr_t isRayTracingAvailablePtr = FindSignature(
		"40 53"
		"48 83 EC 20"
		"48 8B 01"
		"48 8B D9"
		"48 8B 40 08"
		"?? ?? ?? ?? ?? ??"
		"84 C0"
		"74 30"
	);
	if (isRayTracingAvailablePtr) {
		Hook(RayTracingOptions_isRayTracingAvailable, (void*)isRayTracingAvailablePtr);
	} else {
		printf("Failed to hook RayTracingOptions::isRayTracingAvailable\n");
	}

	//dragon::bgfximpl::getShaderCodePlatform
	//1.19.80
	uintptr_t getShaderCodePlatformPtr = FindSignature(
		"48 83 EC 38 "
		"48 63 05 ? ? ? ? "
	);
	if (getShaderCodePlatformPtr) {
		Hook(dragon_bgfximpl_getShaderCodePlatform, (void*)getShaderCodePlatformPtr);
	} else {
		printf("Failed to hook dragon::bgfximpl::getShaderCodePlatform\n");
	}
}
