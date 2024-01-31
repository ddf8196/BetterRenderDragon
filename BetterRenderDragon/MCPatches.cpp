#include "HookAPI.h"
#include "MCPatches.h"
#include "Options.h"

void initMCPatches() {
	if (Options::vanilla2DeferredEnabled && Options::disableRendererContextD3D12RTX) {
		//Deferred rendering no longer requires RendererContextD3D12RTX since 1.19.80, so it can be disabled for better performance
		//bgfx::d3d12rtx::RendererContextD3D12RTX::init
		//1.19.40
		uintptr_t ptr = FindSignature("83 BE ?? 02 00 00 65 ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? 83 BE ?? 02 00 00 65");
		if (ptr) {
			ScopedVP(ptr, 60, PAGE_READWRITE);
			((char*)ptr)[6] = 0x7F;
			((char*)ptr)[59] = 0x7F;
		} else {
			//1.20.30.21 preview
			uintptr_t ptr2 = FindSignature("83 39 65 ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? 83 39 65");
			if (ptr2) {
				ScopedVP(ptr2, 60, PAGE_READWRITE);
				((char*)ptr2)[2] = 0x7F;
				((char*)ptr2)[52] = 0x7F;
			} else {
				printf("Failed to patch bgfx::d3d12rtx::RendererContextD3D12RTX::init\n");
			}
		}
	}

	//Bypass VendorID check to support some Intel GPUs
	//bgfx::d3d12::RendererContextD3D12::init
	//1.19.40
	uintptr_t ptr2 = FindSignature("81 BF ?? ?? 00 00 86 80 00 00");
	if (!ptr2) {
		//1.20.0.23 preview
		ptr2 = FindSignature("81 BE ?? ?? 00 00 86 80 00 00");
	}
	if (ptr2) {
		ScopedVP(ptr2, 10, PAGE_READWRITE);
		((char*)ptr2)[6] = 0;
		((char*)ptr2)[7] = 0;
	} else {
		printf("Failed to patch bgfx::d3d12::RendererContextD3D12::init\n");
	}

	//Fix rendering issues on some NVIDIA GPUs
	//dragon::bgfximpl::toSamplerFlags
	//1.20.0.22 preview
	uintptr_t ptr3 = FindSignature("FF E1 B8 00 00 07 00 C3");
	if (ptr3) {
		ScopedVP(ptr3, 8, PAGE_READWRITE);
		((char*)ptr3)[5] = 0;
	} else {
		printf("Failed to patch dragon::bgfximpl::toSamplerFlags\n");
	}

	//MinecraftGame::_updateLightingModel
	//1.20.30.02
	uintptr_t ptr4 = FindSignature("83 FB 01 75 11 48 8B 01 48 8B 40 40 FF 15 ? ? ? ? 32 C0 EB 02");
	if (ptr4) {
		ScopedVP(ptr4, 22, PAGE_READWRITE);
		((char*)ptr4)[18] = 0xB0;
		((char*)ptr4)[19] = 0x01;
	} else {
		//1.20.30.20 preview
		ptr4 = FindSignature("83 FB 01 75 1A 48 8B 01 48 8B 40 40 FF 15 ? ? ? ? 84 C0 74 05 45 84 E4 75 04");
		if (ptr4) {
			ScopedVP(ptr4, 27, PAGE_READWRITE);
			((char*)ptr4)[3] = 0x74;
		} else {
			printf("Failed to patch MinecraftGame::_updateLightingModel\n");
		}
	}
}
