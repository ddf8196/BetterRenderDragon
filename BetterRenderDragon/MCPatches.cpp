#include "HookAPI.h"
#include "MCPatches.h"
#include "Options.h"

void MCPatches_Init() {
	printf("%s\n", __FUNCTION__);

	if (Options::vanilla2DeferredEnabled && Options::disableRendererContextD3D12RTX) {
		//Deferred rendering no longer requires RendererContextD3D12RTX since 1.19.80, so it can be disabled for better performance
		//bgfx::d3d12rtx::RendererContextD3D12RTX::init
		//1.19.40
		uintptr_t ptr = FindSignature("83 BE ?? 02 00 00 65 ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? 83 BE ?? 02 00 00 65");
		if (ptr) {
			DWORD oldProtect, tmp;
			VirtualProtect((void*)ptr, 60, PAGE_READWRITE, &oldProtect);
			((char*)ptr)[6] = 0x7F;
			((char*)ptr)[59] = 0x7F;
			VirtualProtect((void*)ptr, 60, oldProtect, &tmp);
		}
		else {
			printf("Failed to patch bgfx::d3d12rtx::RendererContextD3D12RTX::init\n");
		}
	}

	//Bypass VendorID check to support Intel GPUs
	//bgfx::d3d12::RendererContextD3D12::init
	//1.19.40
	uintptr_t ptr2 = FindSignature("81 BF ?? ?? 00 00 86 80 00 00");
	if (ptr2) {
		DWORD oldProtect, tmp;
		VirtualProtect((void*)ptr2, 10, PAGE_READWRITE, &oldProtect);
		((char*)ptr2)[6] = 0;
		((char*)ptr2)[7] = 0;
		VirtualProtect((void*)ptr2, 10, oldProtect, &tmp);
	} else {
		printf("Failed to patch bgfx::d3d12::RendererContextD3D12::init\n");
	}

	//Fix rendering issues on some NVIDIA GPUs
	//unkFunc1
	//1.20.0.22 preview
	uintptr_t ptr3 = FindSignature("B8 00 00 07 00 C3");
	if (ptr3) {
		DWORD oldProtect, tmp;
		VirtualProtect((void*)ptr3, 6, PAGE_READWRITE, &oldProtect);
		((char*)ptr3)[3] = 0;
		VirtualProtect((void*)ptr3, 6, oldProtect, &tmp);
	} else {
		printf("Failed to patch unkFunc1\n");
	}
}
