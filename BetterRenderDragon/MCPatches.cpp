#include "HookAPI.h"
#include "MCPatches.h"
#include "Options.h"

#undef FindSignature
#define FindSignature(signature) ((uint8_t*)FindSig("Minecraft.Windows.exe", signature))

void initMCPatches() {
	if (Options::vanilla2DeferredEnabled && Options::disableRendererContextD3D12RTX) {
		//Deferred rendering no longer requires RendererContextD3D12RTX since 1.19.80, so it can be disabled for better performance
		//bgfx::d3d12rtx::RendererContextD3D12RTX::init
		if (auto ptr = FindSignature("83 BE ?? 02 00 00 65 ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? 83 BE ?? 02 00 00 65"); ptr) {
			//1.19.40
			ScopedVP(ptr, 60, PAGE_READWRITE);
			ptr[6] = 0x7F;
			ptr[59] = 0x7F;
		} else if (ptr = FindSignature("83 39 65 ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? 83 39 65"); ptr) {
			//1.20.30.21 preview
			ScopedVP(ptr, 53, PAGE_READWRITE);
			ptr[2] = 0x7F;
			ptr[52] = 0x7F;
		} else {
			printf("Failed to patch bgfx::d3d12rtx::RendererContextD3D12RTX::init\n");
		}
	}

	//Bypass VendorID check to support some Intel GPUs
	//bgfx::d3d12::RendererContextD3D12::init
	if (auto ptr = FindSignature("81 BF ?? ?? 00 00 86 80 00 00"); ptr) {
		//1.19.40
		ScopedVP(ptr, 10, PAGE_READWRITE);
		ptr[6] = 0;
		ptr[7] = 0;
	} else if (ptr = FindSignature("81 BE ?? ?? 00 00 86 80 00 00"); ptr) {
		//1.20.0.23 preview
		ScopedVP(ptr, 10, PAGE_READWRITE);
		ptr[6] = 0;
		ptr[7] = 0;
	} else {
		printf("Failed to patch bgfx::d3d12::RendererContextD3D12::init\n");
	}
	
	//Fix rendering issues on some NVIDIA GPUs
	//dragon::bgfximpl::toSamplerFlags
	if (auto ptr = FindSignature("FF E1 B8 00 00 07 00 C3"); ptr) {
		//1.20.0.22 preview
		ScopedVP(ptr, 8, PAGE_READWRITE);
		ptr[5] = 0;
	} else {
		printf("Failed to patch dragon::bgfximpl::toSamplerFlags\n");
	}
	
	//MinecraftGame::_updateLightingModel
	if (auto ptr = FindSignature("83 FB 01 75 11 48 8B 01 48 8B 40 40 FF 15 ? ? ? ? 32 C0 EB 02"); ptr) {
		//1.20.30.02
		ScopedVP(ptr, 22, PAGE_READWRITE);
		ptr[18] = 0xB0;
		ptr[19] = 0x01;
	} else if (ptr = FindSignature("83 FB 01 75 1A 48 8B 01 48 8B 40 40 FF 15 ? ? ? ? 84 C0 74 05 45 84 E4 75 04"); ptr) {
		//1.20.30.20 preview
		ScopedVP(ptr, 27, PAGE_READWRITE);
		ptr[3] = 0x74;
	} else {
		printf("Failed to patch MinecraftGame::_updateLightingModel\n");
	}
}
