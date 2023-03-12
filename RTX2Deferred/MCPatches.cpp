#include "HookAPI.h"
#include "MCPatches.h"

void MCPatches_Init() {
	printf("%s\n", __FUNCTION__);

	//bgfx::d3d12rtx::RendererContextD3D12RTX::init
	uintptr_t ptr = FindSignature("83 BE D4 02 00 00 65 ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? 83 BE D4 02 00 00 65");
	if (ptr) {
		DWORD oldProtect, tmp;
		VirtualProtect((void*)ptr, 60, PAGE_READWRITE, &oldProtect);
		((char*)ptr)[6] = 0x51;
		((char*)ptr)[59] = 0x51;
		VirtualProtect((void*)ptr, 60, oldProtect, &tmp);
	} else {
		printf("Failed to patch bgfx::d3d12rtx::RendererContextD3D12RTX::init\n");
	}

	//bgfx::d3d12::RendererContextD3D12::init
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
}
