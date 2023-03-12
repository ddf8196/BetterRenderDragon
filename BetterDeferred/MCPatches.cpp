#include "HookAPI.h"
#include "MCPatches.h"

#include "mce.h"

void MCPatches_Init() {
	printf("%s\n", __FUNCTION__);

	//mce::framebuilder::bgfxbridge::DeferredMinecraftFrameRenderer::frame
	{
		mce::TextureFormat* textureFormatPtr = nullptr;
		uintptr_t ptr = FindSignature(
			"48 8D 8D ?? ?? ?? ?? "
			"E8 ?? ?? ?? ?? "
			"0F 10 B5 ?? ?? ?? ?? "
			"0F 11 B5 ?? ?? ?? ?? "
			"8B 9D ?? ?? ?? ?? "
			"89 9D ?? ?? ?? ?? "
			"C7 85 ?? ?? ?? ?? ?? ?? ?? ??"
		);
		textureFormatPtr = (mce::TextureFormat*)(ptr + 44);

		if (!ptr) {
			ptr = FindSignature(
				"48 8D 8D ?? ?? ?? ?? "
				"E8 ?? ?? ?? ?? "
				"0F 10 ?? ?? ?? ?? ?? "
				"0F 11 ?? ?? ?? ?? ?? "
				"F2 0F 10 ?? ?? ?? ?? ?? "
				"F2 0F 11 ?? ?? ?? ?? ?? "
				"8B 9D ?? ?? ?? ?? "
				"89 9D ?? ?? ?? ?? "
				"C7 85 ?? ?? ?? ?? ?? ?? ?? ??"
			);
			textureFormatPtr = (mce::TextureFormat*)(ptr + 60);
		}

		if (ptr && textureFormatPtr) {
			DWORD oldProtect, tmp;
			VirtualProtect(textureFormatPtr, sizeof(mce::TextureFormat), PAGE_READWRITE, &oldProtect);

			mce::TextureFormat textureFormat = *textureFormatPtr;
			if (textureFormat == mce::TextureFormat::R16G16_SNORM || textureFormat == mce::TextureFormat::R8G8_UNORM) {
				*textureFormatPtr = mce::TextureFormat::R16G16B16A16_FLOAT;
			} else {
				printf("Failed to patch mce::framebuilder::bgfxbridge::DeferredMinecraftFrameRenderer::frame\n");
			}

			VirtualProtect(textureFormatPtr, sizeof(mce::TextureFormat), oldProtect, &tmp);
		} else {
			printf("Failed to patch mce::framebuilder::bgfxbridge::DeferredMinecraftFrameRenderer::frame\n");
		}
	}

}

