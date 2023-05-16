#include <unordered_set>

#include "HookAPI.h"
#include "MCHooks.h"
#include "Options.h"

#include "Renderdragon/Materials/ShaderCodePlatform.h"
#include "Renderdragon/Materials/MaterialUniformName.h"
#include "Renderdragon/Rendering/LightingModels.h"
#include "Core/Resource/ResourceHelper.h"
#include "gsl/span"

using dragon::rendering::LightingModels;

LightingModels(*RayTracingOptions_getLightingModel)(void* This) = nullptr;
LightingModels RayTracingOptions_getLightingModel_Hook(void* This) {
	LightingModels result = RayTracingOptions_getLightingModel(This);
	if (result == LightingModels::Vanilla && Options::deferredRenderingEnabled) {
		result = LightingModels::Deferred;
	}
	return result;
}

void(*RayTracingOptions_setLightingModel)(void* This, LightingModels lightingModel) = nullptr;
void RayTracingOptions_setLightingModel_Hook(void* This, LightingModels lightingModel) {
	if (lightingModel == LightingModels::Vanilla && Options::deferredRenderingEnabled) {
		lightingModel = LightingModels::Deferred;
	}
	RayTracingOptions_setLightingModel(This, lightingModel);
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
	if (result == ShaderCodePlatform::Direct3D_SM65 && Options::deferredRenderingEnabled && Options::limitShaderModel) {
		result = ShaderCodePlatform::Direct3D_SM60;
	}
	return result;
}

using dragon::materials::MaterialUniformName;
DeclareHook(dragon_materials_MaterialUniformMap_setUniform_mun_vec4, void*, void* This, void* outParameterID, MaterialUniformName& name, gsl::span<Vec4>* value) {
	return original(This, outParameterID, name, value);
}

typedef bool(*PFN_ResourcePackManager_load)(void* This, const ResourceLocation& location, std::string& resourceStream);

void* resourcePackManager;
PFN_ResourcePackManager_load ResourcePackManager_load;
DeclareHook(ResourcePackManager_constructor, void*, void* This, uintptr_t a2, uintptr_t a3, bool needsToInitialize) {
	printf("ResourcePackManager::ResourcePackManager needsToInitialize=%s\n", needsToInitialize ? "true" : "false");

	void* result = original(This, a2, a3, needsToInitialize);
	if (needsToInitialize && !resourcePackManager) {
		resourcePackManager = This;
		void** vptr = *(void***)resourcePackManager;
		ResourcePackManager_load = (PFN_ResourcePackManager_load)*(vptr + 3);
	}

	return result;
}

DeclareHook(readFile, std::string*, void* This, std::string* retstr, Core::Path& path) {
	std::string* result = original(This, retstr, path);
	if (Options::redirectShaders && resourcePackManager) {
		std::string& p = path.mPathPart.mUtf8StdString;
		if (p.find("/data/renderer/materials/") != std::string::npos && strncmp(p.c_str() + p.size() - 13, ".material.bin", 13) == 0) {
			std::string binPath = "renderer/materials/" + p.substr(p.find_last_of('/') + 1);
			Core::Path path1(binPath);
			ResourceLocation location(path1);
			std::string out;

			bool success = ResourcePackManager_load(resourcePackManager, location, out);
			if (success) {
				printf("ResourcePackManager::load Success location=%s length=%lld\n", binPath.c_str(), out.length());
				retstr->assign(out);
			} else {
				printf("ResourcePackManager::load Failure location=%s\n", binPath.c_str());
			}
		}
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
		"48 63 05 ? ? ? ? "
		"83 F8 0A "
		"0F ? ? ? ? ?"
	);
	if (!getShaderCodePlatformPtr) {
		//1.19.80 preview
		getShaderCodePlatformPtr = FindSignature(
			"48 83 EC 38 "
			"48 63 05 ? ? ? ? "
		);
	}
	if (getShaderCodePlatformPtr) {
		Hook(dragon_bgfximpl_getShaderCodePlatform, (void*)getShaderCodePlatformPtr);
	} else {
		printf("Failed to hook dragon::bgfximpl::getShaderCodePlatform\n");
	}

	//dragon::materials::MaterialUniformMap::setUniform<glm::vec4>
	//1.19.40
	uintptr_t setUniformPtr = NULL;
	uintptr_t call_setUniformPtr = FindSignature(
		"E8 ? ? ? ? "
		"F3 41 0F 10 96 ? ? ? ? "
	);
	if (call_setUniformPtr) {
		setUniformPtr = call_setUniformPtr + 5 + *(int32_t*)(call_setUniformPtr + 1);
	}
	if (setUniformPtr) {
		Hook(dragon_materials_MaterialUniformMap_setUniform_mun_vec4, (void*)setUniformPtr);
	} else {
		printf("Failed to hook dragon::materials::MaterialUniformMap::setUniform<glm::vec4>\n");
	}

	//ResourcePackManager::ResourcePackManager
	//1.19.80
	uintptr_t resourcePackManagerPtr = FindSignature(
		"48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 41 0F B6 F1 49 8B D8 4C 8B F2 48 8B F9"
	);
	if (resourcePackManagerPtr) {
		Hook(ResourcePackManager_constructor, (void*)resourcePackManagerPtr);
	} else {
		printf("Failed to hook ResourcePackManager::ResourcePackManager\n");
	}

	//1.19.40
	uintptr_t readFilePtr = FindSignature(
		"48 89 5C 24 ? 55 56 57 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 45 70 49 8B C0"
	);
	if (!readFilePtr)
		//1.20.0.23 preview
		readFilePtr = FindSignature(
			"48 89 5C 24 08 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 D0"
		);
	if (readFilePtr) {
		Hook(readFile, (void*)readFilePtr);
	} else {
		printf("Failed to hook readFile\n");
	}
}
