#include <unordered_set>

#include "HookAPI.h"
#include "MCHooks.h"
#include "Options.h"

#include "Renderdragon/Materials/ShaderCodePlatform.h"
#include "Renderdragon/Materials/MaterialUniformName.h"
#include "Renderdragon/Rendering/LightingModels.h"
#include "Core/Resource/ResourceHelper.h"
#include "Core/Math/Vec4.h"
#include "gsl/span"

//=====================================================Vanilla2Deferred=====================================================

bool shouldForceEnableDeferredRendering() {
	return Options::vanilla2DeferredAvailable && Options::vanilla2DeferredEnabled && !Options::newVideoSettingsAvailable && Options::deferredRenderingEnabled;
}

bool shouldForceEnableNewVideoSettings() {
	return Options::vanilla2DeferredAvailable && Options::vanilla2DeferredEnabled && Options::newVideoSettingsAvailable && Options::forceEnableDeferredTechnicalPreview;
}

bool(*RayTracingOptions_isDeferredShadingAvailable)(void* This) = nullptr;
bool RayTracingOptions_isDeferredShadingAvailable_Hook(void* This) {
	if (shouldForceEnableNewVideoSettings()) {
		return true;
	}
	return RayTracingOptions_isDeferredShadingAvailable(This);
}

using dragon::rendering::LightingModels;

LightingModels(*RayTracingOptions_getLightingModel)(void* This) = nullptr;
LightingModels RayTracingOptions_getLightingModel_Hook(void* This) {
	LightingModels result = RayTracingOptions_getLightingModel(This);
	//printf("RayTracingOptions::getLightingModel result=%d\n", result);

	if (shouldForceEnableDeferredRendering() && result == LightingModels::Vanilla) {
		result = LightingModels::Deferred;
	}
	return result;
}

void(*RayTracingOptions_setLightingModel)(void* This, LightingModels lightingModel) = nullptr;
void RayTracingOptions_setLightingModel_Hook(void* This, LightingModels lightingModel) {
	//printf("RayTracingOptions::setLightingModel lightingModel=%d\n", lightingModel);

	if (shouldForceEnableDeferredRendering() && lightingModel == LightingModels::Vanilla) {
		lightingModel = LightingModels::Deferred;
	}
	
	RayTracingOptions_setLightingModel(This, lightingModel);
}

DeclareHook(RayTracingOptions_isRayTracingAvailable, bool, void* This) {
	printf("RayTracingOptions::isRayTracingAvailable\n");

	ReplaceVtable(*(void**)This, 8, (void**)&RayTracingOptions_isDeferredShadingAvailable, RayTracingOptions_isDeferredShadingAvailable_Hook);
	ReplaceVtable(*(void**)This, 9, (void**)&RayTracingOptions_getLightingModel, RayTracingOptions_getLightingModel_Hook);
	ReplaceVtable(*(void**)This, 10, (void**)&RayTracingOptions_setLightingModel, RayTracingOptions_setLightingModel_Hook);
	bool result = original(This);
	Unhook(RayTracingOptions_isRayTracingAvailable);
	return result;
}

DeclareHook(isLightingModelSupported, bool, void* a1, LightingModels lightingModel, char a3) {
	if (shouldForceEnableNewVideoSettings() && lightingModel == LightingModels::Deferred)
		return true;
	return original(a1, lightingModel, a3);
}

DeclareHook(graphicsModeRadioDeferredEnabledCallback, bool, void* a1) {
	bool result = original(a1);
	if (shouldForceEnableNewVideoSettings())
		result = true;
	return result;
}

void* newVideoSettingsOptionPtr = nullptr;
DeclareHook(makeBoolOption, void**, void** a1, uintptr_t a2, uintptr_t a3, uintptr_t a4, const char* a5, const char* a6, uintptr_t a7) {
	void** result = original(a1, a2, a3, a4, a5, a6, a7);
	if (!newVideoSettingsOptionPtr && a1 && a5 && a6 && strcmp(a5, "options.newVideoSettings") == 0 && strcmp(a6, "new_video_settings") == 0) {
		//printf("options.newVideoSettings\n");
		newVideoSettingsOptionPtr = *a1;
		Options::newVideoSettingsAvailable = true;
	}
	return result;
}

DeclareHook(Option_getBool, bool, void* This) {
	if (shouldForceEnableNewVideoSettings() && This == newVideoSettingsOptionPtr) {
		//printf("Option::getBool\n");
		return true;
	}
	return original(This);
}

DeclareHook(BoolOption_set, void, void* This, bool value, char a3) {
	if (shouldForceEnableNewVideoSettings() && This == newVideoSettingsOptionPtr) {
		//printf("BoolOption::set\n");
		value = true;
	}
	original(This, value, a3);
}

DeclareHook(Option_set_bool, void, void* This, bool* value) {
	if (shouldForceEnableNewVideoSettings() && This == newVideoSettingsOptionPtr) {
		//printf("Option::set<bool>\n");
		*value = true;
	}
	original(This, value);
}

//======================================================CustomUniforms======================================================

using dragon::materials::MaterialUniformName;
DeclareHook(dragon_materials_MaterialUniformMap_setUniform_mun_vec4, void*, void* This, void* outParameterID, MaterialUniformName& name, gsl::span<Vec4>* value) {
	return original(This, outParameterID, name, value);
}

//=====================================================MaterialBinLoader====================================================

typedef bool(*PFN_ResourcePackManager_load)(void* This, const ResourceLocation& location, std::string& resourceStream);

void* resourcePackManager;
PFN_ResourcePackManager_load ResourcePackManager_load;
DeclareHook(ResourcePackManager_constructor, void*, void* This, uintptr_t a2, uintptr_t a3, bool needsToInitialize) {
	void* result = original(This, a2, a3, needsToInitialize);
	if (needsToInitialize && !resourcePackManager) {
		printf("ResourcePackManager::ResourcePackManager needsToInitialize=true\n");

		resourcePackManager = This;
		void** vptr = *(void***)resourcePackManager;
		ResourcePackManager_load = (PFN_ResourcePackManager_load)*(vptr + 3);
	}
	return result;
}

DeclareHook(readFile, std::string*, void* This, std::string* retstr, Core::Path& path) {
	std::string* result = original(This, retstr, path);
	if (Options::materialBinLoaderEnabled && Options::redirectShaders && resourcePackManager) {
		std::string& p = path.mPathPart.mUtf8StdString;
		if (p.find("/data/renderer/materials/") != std::string::npos && strncmp(p.c_str() + p.size() - 13, ".material.bin", 13) == 0) {
			std::string binPath = "renderer/materials/" + p.substr(p.find_last_of('/') + 1);
			Core::Path path1(binPath);
			ResourceLocation location(path1);
			std::string out;
			//printf("ResourcePackManager::load path=%s\n", binPath.c_str());

			bool success = ResourcePackManager_load(resourcePackManager, location, out);
			if (success) {
				retstr->assign(out);
			}
			//printf("ResourcePackManager::load ret=%d\n", success);
		}
	}
	return result;
}

//==========================================================================================================================

void MCHooks_Init() {
	printf("%s\n", __FUNCTION__);

	//RayTracingOptions::isRayTracingAvailable
	//1.19.80
	uintptr_t isRayTracingAvailablePtr = FindSignature("40 53 48 83 EC 20 48 8B 01 48 8B D9 48 8B 40 08 ? ? ? ? ? ? 84 C0 74 30");
	if (isRayTracingAvailablePtr) {
		Hook(RayTracingOptions_isRayTracingAvailable, (void*)isRayTracingAvailablePtr);
	} else {
		printf("Failed to hook RayTracingOptions::isRayTracingAvailable\n");
	}

	/*Force Enable Deferred Technical Preview*/ {
		//std::make_unique<BoolOption>
		//1.20.30.02
		//1.20.30.21 preview
		uintptr_t makeBoolOptionPtr = FindSignature("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 4D 8B E1 4D 8B E8 48 89 54 24 ? 4C 8B F1 48 89 4C 24 ? 48 8B AC 24 ? ? ? ? 48 8B B4 24 ? ? ? ? 33 FF");
		if (!makeBoolOptionPtr) {
			//1.20.30.20 preview
			makeBoolOptionPtr = FindSignature("48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 55 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 4C 89 4D F7 4C 89 45 FF 48 89 55 07 4C 8B E1 48 89 4D 0F 4C 8B 7D 6F 4C 8B 75 77 33 FF");
		}
		if (!makeBoolOptionPtr) {
			//1.20.50.20 preview
			makeBoolOptionPtr = FindSignature("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 4D 8B E1 4D 8B E8 48 89 54 24 ? 4C 8B F1 48 89 4C 24 ? 48 8B AC 24 ? ? ? ? 48 8B B4 24 ? ? ? ? 33 DB 89 5C 24 40 E8 ? ? ? ? 48 8B C8 48 8B 00 8D 53 58");
		}
		if (makeBoolOptionPtr) {
			Hook(makeBoolOption, (void*)makeBoolOptionPtr);
		} else {
			printf("Failed to hook std::make_unique<BoolOption>\n");
		}

		//Option::getBool
		//1.20.30.02
		uintptr_t optionGetBoolPtr = FindSignature("48 8B 41 08 48 8B 90 ? ? ? ? 48 85 D2 74 18");
		if (optionGetBoolPtr) {
			Hook(Option_getBool, (void*)optionGetBoolPtr);
		} else {
			printf("Failed to hook Option::getBool\n");
		}
	 
		//BoolOption::set
		//1.20.30.02
		//1.20.30.21 preview
		uintptr_t boolOptionSetPtr = FindSignature("48 89 5C 24 ? 57 48 83 EC 30 48 8B 05 ? ? ? ? 48 33 C4 48 89 44 24 ? 41 0F B6 F8 0F B6 C2 48 8B D9");
		if (boolOptionSetPtr) {
			Hook(BoolOption_set, (void*)boolOptionSetPtr);
		}

		//Option::set<bool>
		//1.20.30.20 preview
		uintptr_t optionsSetBoolPtr = FindSignature("48 89 5C 24 ? 57 48 83 EC 40 48 8B 41 08 48 8B FA 48 8B D9 83 B8 ? ? ? ? ? 74 3C 48 8D 05 ? ? ? ? 41 B9 ? ? ? ?");
		if (optionsSetBoolPtr) {
			Hook(Option_set_bool, (void*)optionsSetBoolPtr);
		}

		if (!boolOptionSetPtr && !optionsSetBoolPtr) {
			printf("Failed to hook BoolOption::set or Option::set<bool>\n");
		}

		//1.20.30.02
		uintptr_t isLightingModelSupportedPtr = FindSignature("40 53 48 83 EC 20 41 0F B6 D8 83 FA 02 75 13 48 8B 01 48 8B 40 38 48 83 C4 20 5B 48 FF 25 ? ? ? ?");
		if (isLightingModelSupportedPtr) {
			Hook(isLightingModelSupported, (void*)isLightingModelSupportedPtr);
		} else {
			printf("Failed to hook isLightingModelSupported\n");
		}

		//1.20.30.02
		uintptr_t callbackPtr = FindSignature("48 89 5C 24 ? 57 48 83 EC 40 48 8B D9 C7 44 24 ? ? ? ? ? 48 8B 01 48 8B 88 ? ? ? ? 48 85 C9");
		if (!callbackPtr) {
			//1.20.30.20 preview
			callbackPtr = FindSignature("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC 60 C7 44 24 ? ? ? ? ?");
		}
		if (!callbackPtr) {
			//1.20.30.21 preview
			callbackPtr = FindSignature("48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 54 41 56 41 57 48 83 EC 60 C7 44 24 ? ? ? ? ?");
		}
		if (!callbackPtr) {
			//1.20.50.20 preview
			callbackPtr = FindSignature("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC 60 C7 44 24 ? ? ? ? ?");
		}
		if (callbackPtr) {
			Hook(graphicsModeRadioDeferredEnabledCallback, (void*)callbackPtr);
		} else {
			printf("Failed to hook graphicsModeRadioDeferredEnabledCallback\n");
		}
	}

	////dragon::materials::MaterialUniformMap::setUniform<glm::vec4>
	////1.19.40
	//uintptr_t setUniformPtr = NULL;
	//uintptr_t call_setUniformPtr = FindSignature(
	//	"E8 ? ? ? ? "
	//	"F3 41 0F 10 96 ? ? ? ? "
	//);
	//if (call_setUniformPtr) {
	//	setUniformPtr = call_setUniformPtr + 5 + *(int32_t*)(call_setUniformPtr + 1);
	//}
	//if (setUniformPtr) {
	//	Hook(dragon_materials_MaterialUniformMap_setUniform_mun_vec4, (void*)setUniformPtr);
	//} else {
	//	printf("Failed to hook dragon::materials::MaterialUniformMap::setUniform<glm::vec4>\n");
	//}

	//ResourcePackManager::ResourcePackManager
	//1.19.80
	uintptr_t resourcePackManagerPtr = FindSignature("48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 41 0F B6 F1 49 8B D8 4C 8B F2 48 8B F9");
	if (!resourcePackManagerPtr) {
		//1.20.1.02
		resourcePackManagerPtr = FindSignature("48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 41 0F B6 F1 49 8B F8 4C 8B F2 48 8B D9");
	}
	if (!resourcePackManagerPtr) {
		//1.20.10.23 preview
		resourcePackManagerPtr = FindSignature("48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 45 0F B6 F1 49 8B F8 48 8B F2 48 8B D9");
	}
	if (!resourcePackManagerPtr) {
		//1.20.30.21 preview
		resourcePackManagerPtr = FindSignature("4C 8B DC 53 55 56 57 41 56 48 81 EC ? ? ? ? 41 0F B6 E9 49 8B D8 48 8B F2 48 8B F9");
	}
	if (resourcePackManagerPtr) {
		Hook(ResourcePackManager_constructor, (void*)resourcePackManagerPtr);
	} else {
		printf("Failed to hook ResourcePackManager::ResourcePackManager\n");
	}

	//1.19.40
	uintptr_t readFilePtr = FindSignature("48 89 5C 24 ? 55 56 57 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 45 70 49 8B C0");
	if (!readFilePtr) {
		//1.20.0.23 preview
		readFilePtr = FindSignature("48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 45 20 49 8B C0");
	}
	if (!readFilePtr) {
		//1.20.30.21 preview
		readFilePtr = FindSignature("48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 49 8B C0 48 8B FA");
	}
	if (!readFilePtr) {
		//1.20.30.02
		readFilePtr = FindSignature("48 89 5C 24 ? 55 56 57 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 49 8B C0");
	}

	if (readFilePtr) {
		Hook(readFile, (void*)readFilePtr);
	} else {
		printf("Failed to hook readFile\n");
	}
}
