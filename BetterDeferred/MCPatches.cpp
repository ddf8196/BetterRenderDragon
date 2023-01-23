#include "HookAPI.h"
#include "MCPatches.h"

#include "mce.h"

void MCPatches_Init() {
	printf("%s\n", __FUNCTION__);

	//mce::framebuilder::bgfxbridge::DeferredMinecraftFrameRenderer::frame
	uintptr_t normalsRasterTextureFormatPtr = FindSignature("48 8D 8D ?? ?? ?? ?? E8 ?? ?? ?? ?? 0F 10 B5 ?? ?? ?? ?? 0F 11 B5 ?? ?? ?? ?? 8B 9D ?? ?? ?? ?? 89 9D ?? ?? ?? ?? C7 85 ?? ?? ?? ?? ?? ?? ?? ??");
	if (normalsRasterTextureFormatPtr) {
		DWORD oldProtect, tmp;
		VirtualProtect((void*)normalsRasterTextureFormatPtr, 48, PAGE_READWRITE, &oldProtect);
		char* p = (char*)normalsRasterTextureFormatPtr;
		if (p[44] == (char)mce::TextureFormat::R16G16_SNORM || p[44] == (char)mce::TextureFormat::R8G8_UNORM) {
			p[44] = (char)mce::TextureFormat::R16G16B16A16_FLOAT;
		} else {
			printf("Failed to patch mce::framebuilder::bgfxbridge::DeferredMinecraftFrameRenderer::frame\n");
		}
		VirtualProtect((void*)normalsRasterTextureFormatPtr, 48, oldProtect, &tmp);
	} else {
		printf("Failed to patch mce::framebuilder::bgfxbridge::DeferredMinecraftFrameRenderer::frame\n");
	}
}

