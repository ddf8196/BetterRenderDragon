#include "HookAPI.h"
#include "MCHooks.h"

void (*RayTracingOptions_setLightingModel)(void* self, int lightingModel) = nullptr;
void RayTracingOptions_setLightingModel_Hook(void* self, int lightingModel) {
	if (lightingModel == 2) { //RTX
		lightingModel = 1; //Deferred
	}
	RayTracingOptions_setLightingModel(self, lightingModel);
}

DeclareHook(RayTracingOptions_isRayTracingAvailable, bool, void* self) {
	printf("RayTracingOptions::isRayTracingAvailable\n");
	bool result = original(self);

	Unhook(RayTracingOptions_isRayTracingAvailable);
	ReplaceVtable(*(void**)self, 8, (void**)&RayTracingOptions_setLightingModel, RayTracingOptions_setLightingModel_Hook);
	RayTracingOptions_setLightingModel(self, 1);
	return result;
}

DeclareHook(bgfx_d3d12rtx_RendererContextD3D12RTX_isSupportedRTXCard, bool, void* self) {
	printf("bgfx::d3d12rtx::RendererContextD3D12RTX::isSupportedRTXCard\n");
	return true;
}

void MCHooks_Init() {
	printf("%s\n", __FUNCTION__);

	//RayTracingOptions::isRayTracingAvailable
	uintptr_t isRayTracingAvailablePtr = FindSignature("40 53 48 83 EC 20 48 8B 01 48 8B D9 FF 50 08 84 C0 74 22 48 8B 03 48 8B CB FF 50 10 84 C0 74 15 48 8B 03 48 8B CB FF 50 18 84 C0 74 08 B0 01 48 83 C4 20 5B C3 32 C0 48 83 C4 20 5B C3");
	if (!isRayTracingAvailablePtr) {
		isRayTracingAvailablePtr = FindSignature("40 53 48 83 EC 20 48 8B 01 48 8B D9 48 8B 40 08 ?? ?? ?? ?? ?? ?? 84 C0 74 30 48 8B 03 48 8B CB 48 8B 40 10 ?? ?? ?? ?? ?? ?? 84 C0 74 1C 48 8B 03 48 8B CB 48 8B 40 18 ?? ?? ?? ?? ?? ?? 84 C0 74 08 B0 01 48 83 C4 20 5B C3 32 C0 48 83 C4 20 5B C3");
	}
	if (!isRayTracingAvailablePtr) {
		isRayTracingAvailablePtr = FindSignature("40 53 48 83 EC 20 48 8B 01 48 8B D9 48 8B 40 08 ?? ?? ?? ?? ?? ?? 84 C0 74 30 48 8B 03 48 8B CB 48 8B 40 28 ?? ?? ?? ?? ?? ?? 84 C0 74 1C 48 8B 03 48 8B CB 48 8B 40 10 ?? ?? ?? ?? ?? ?? 84 C0 74 08 B0 01 48 83 C4 20 5B C3 32 C0 48 83 C4 20 5B C3 ");
	}
	if (isRayTracingAvailablePtr) {
		Hook(RayTracingOptions_isRayTracingAvailable, (void*)isRayTracingAvailablePtr);
	} else {
		printf("Failed to hook RayTracingOptions::isRayTracingAvailable\n");
	}

	//bgfx::d3d12rtx::RendererContextD3D12RTX::isSupportedRTXCard
	uintptr_t isSupportedRTXCardPtr = FindSignature("48 89 5C 24 08 55 48 8D 6C 24 F0 48 81 EC 10 01 00 00 48 8B D9 B9 ?? 00 00 00 65 48 8B 04 25 58 00 00 00 48 8B 10 8B 04 11");
	if (!isSupportedRTXCardPtr) {
		isSupportedRTXCardPtr = FindSignature("48 89 5C 24 08 48 89 74 24 10 48 89 7C 24 18 55 41 54 41 55 41 56 41 57 48 8D 6C 24 A0 48 81 EC 60 01 00 00 4C 8B F1");
	}
	if (isSupportedRTXCardPtr) {
		Hook(bgfx_d3d12rtx_RendererContextD3D12RTX_isSupportedRTXCard, (void*)isSupportedRTXCardPtr);
	} else {
		printf("Failed to hook bgfx::d3d12rtx::RendererContextD3D12RTX::isSupportedRTXCard\n");
	}
}
