#include "HookAPI.h"
#include "MCHooks.h"
#include "Options.h"

#include "Renderdragon/Materials/ShaderCodePlatform.h"
#include "Renderdragon/Materials/MaterialUniformName.h"
#include "RenderDragon/Materials/MaterialResourceManager.h"
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
		const std::string& p = path.getUtf8StdString();
		if (p.find("/data/renderer/materials/") != std::string::npos && strncmp(p.c_str() + p.size() - 13, ".material.bin", 13) == 0) {
			std::string binPath = "renderer/materials/" + p.substr(p.find_last_of('/') + 1);
			ResourceLocation location(binPath);
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

using dragon::materials::MaterialResourceManager;

typedef void (*PFN_mce_framebuilder_BgfxFrameBuilder_discardFrame)(uintptr_t This, bool waitForPreviousFrame);
typedef void (*PFN_dragon_materials_CompiledMaterialManager_freeShaderBlobs)(uintptr_t This);

PFN_mce_framebuilder_BgfxFrameBuilder_discardFrame discardFrame = nullptr;
PFN_dragon_materials_CompiledMaterialManager_freeShaderBlobs freeShaderBlobs = nullptr;

int offsetToMaterialsManager = -1;

bool discardFrameAndClearShaderCaches(uintptr_t bgfxFrameBuilder) {
	uintptr_t compiledMaterialManager = *(uintptr_t*)(*(uintptr_t*)(bgfxFrameBuilder + 40) + 16) + 768;
	uintptr_t mExtractor = *(uintptr_t*)(bgfxFrameBuilder + 32);
	MaterialResourceManager* mMaterialsManager = *(MaterialResourceManager**)(mExtractor + offsetToMaterialsManager);

	if (discardFrame && freeShaderBlobs && mMaterialsManager) {
		discardFrame(bgfxFrameBuilder, true);

		mMaterialsManager->forceTrim();
		freeShaderBlobs(compiledMaterialManager);
		freeShaderBlobs(compiledMaterialManager);

		return true;
	}
	return false;
}

DeclareHook(mce_framebuilder_BgfxFrameBuilder_endFrame, void, uintptr_t This, uintptr_t frameBuilderContext) {
	bool clear = false;
	if (Options::reloadShadersAvailable && Options::reloadShaders) {
		Options::reloadShaders = false;
		clear = true;
	}
	if (clear && discardFrameAndClearShaderCaches(This)) {
		return;
	}
	original(This, frameBuilderContext);
}

//==========================================================================================================================

void initMCHooks() {
	//RayTracingOptions::isRayTracingAvailable
	TrySigHook(RayTracingOptions_isRayTracingAvailable,
		//1.19.80
		"40 53 48 83 EC 20 48 8B 01 48 8B D9 48 8B 40 08 ? ? ? ? ? ? 84 C0 74 30"
	);

	/*Force Enable Deferred Technical Preview*/ {
		//std::make_unique<BoolOption>
		TrySigHook(makeBoolOption,
			//1.20.30.02
			//1.20.30.21 preview
			"48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 4D 8B E1 4D 8B E8 48 89 54 24 ? 4C 8B F1 48 89 4C 24 ? 48 8B AC 24 ? ? ? ? 48 8B B4 24 ? ? ? ? 33 FF",
			//1.20.30.20 preview
			"48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 55 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 4C 89 4D F7 4C 89 45 FF 48 89 55 07 4C 8B E1 48 89 4D 0F 4C 8B 7D 6F 4C 8B 75 77 33 FF",
			//1.20.50.20 preview
			"48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 4D 8B E1 4D 8B E8 48 89 54 24 ? 4C 8B F1 48 89 4C 24 ? 48 8B AC 24 ? ? ? ? 48 8B B4 24 ? ? ? ? 33 DB 89 5C 24 40 E8 ? ? ? ? 48 8B C8 48 8B 00 8D 53 58"
		);

		//Option::getBool
		TrySigHook(Option_getBool,
			//1.20.30.02
			"48 8B 41 08 48 8B 90 ? ? ? ? 48 85 D2 74 18"
		);
	 
		//BoolOption::set
		bool boolOptionSetHooked = TrySigHookNoWarning(BoolOption_set,
			//1.20.30.02
			//1.20.30.21 preview
			"48 89 5C 24 ? 57 48 83 EC 30 48 8B 05 ? ? ? ? 48 33 C4 48 89 44 24 ? 41 0F B6 F8 0F B6 C2 48 8B D9"
		);

		//Option::set<bool>
		bool optionsSetBoolHooked = TrySigHookNoWarning(Option_set_bool,
			//1.20.30.20 preview
			"48 89 5C 24 ? 57 48 83 EC 40 48 8B 41 08 48 8B FA 48 8B D9 83 B8 ? ? ? ? ? 74 3C 48 8D 05 ? ? ? ? 41 B9 ? ? ? ?"
		);

		if (!boolOptionSetHooked && !optionsSetBoolHooked) {
			printf("Failed to hook BoolOption::set or Option::set<bool>\n");
		}

		TrySigHook(isLightingModelSupported,
			//1.20.30.02
			"40 53 48 83 EC 20 41 0F B6 D8 83 FA 02 75 13 48 8B 01 48 8B 40 38 48 83 C4 20 5B 48 FF 25 ? ? ? ?"
		);

		TrySigHook(graphicsModeRadioDeferredEnabledCallback,
			//1.20.30.02
			"48 89 5C 24 ? 57 48 83 EC 40 48 8B D9 C7 44 24 ? ? ? ? ? 48 8B 01 48 8B 88 ? ? ? ? 48 85 C9",
			//1.20.30.20 preview
			"48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC 60 C7 44 24 ? ? ? ? ?",
			//1.20.30.21 preview
			"48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 54 41 56 41 57 48 83 EC 60 C7 44 24 ? ? ? ? ?",
			//1.20.50.20 preview
			"48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC 60 C7 44 24 ? ? ? ? ?"
		);
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
	TrySigHook(ResourcePackManager_constructor,
		//1.19.80
		"48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 41 0F B6 F1 49 8B D8 4C 8B F2 48 8B F9",
		//1.20.1.02
		"48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 41 0F B6 F1 49 8B F8 4C 8B F2 48 8B D9",
		//1.20.10.23 preview
		"48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 45 0F B6 F1 49 8B F8 48 8B F2 48 8B D9",
		//1.20.30.21 preview
		"4C 8B DC 53 55 56 57 41 56 48 81 EC ? ? ? ? 41 0F B6 E9 49 8B D8 48 8B F2 48 8B F9",
		//1.21.20.24 preview
		"4C 8B DC 53 55 56 57 41 56 48 81 EC ? ? ? ? 41 0F B6 E9 49 8B F0 48 8B FA 48 8B D9"
	);

	TrySigHook(readFile,
		//1.19.40
		"48 89 5C 24 ? 55 56 57 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 45 70 49 8B C0",
		//1.20.0.23 preview
		"48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 45 20 49 8B C0",
		//1.20.30.21 preview
		"48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 49 8B C0 48 8B FA",
		//1.20.30.02
		"48 89 5C 24 ? 55 56 57 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 49 8B C0 48 8B F2 48 89 95 ? ? ? ?",
		//1.20.71.01
		"48 89 5C 24 ? 55 56 57 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 49 8B C0 48 8B FA 48 89 95 ? ? ? ?",
		//1.20.80.20 preview
		"48 89 5C 24 ? 48 89 74 24 ? 55 57 41 54 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 45 ? 49 8B C0 48 8B FA",
		//1.20.80.21 preview
		"48 89 5C 24 ? 48 89 74 24 ? 55 57 41 54 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 49 8B C0",
		//1.20.80.05
		"48 89 5C 24 ? 55 56 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 49 8B C0 48 8B FA",
		//1.21.20.24 preview
		"48 89 5C 24 ? 55 56 57 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 49 8B F8 48 8B F2 48 89 55 ? ? ? ?"
	);

	if (TrySigHookNoWarning(mce_framebuilder_BgfxFrameBuilder_endFrame, "48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? B8 ? ? ? ? E8 ? ? ? ? 48 2B E0 0F 29 B4 24 ? ? ? ? 0F 29 BC 24 ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 4C 8B FA 48 89 55 ? 4C 8B F1")) {
		//1.20.30.02
		offsetToMaterialsManager = 768;
	} else if (TrySigHookNoWarning(mce_framebuilder_BgfxFrameBuilder_endFrame, 
		//1.20.60.04
		"48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? B8 ? ? ? ? E8 ? ? ? ? 48 2B E0 0F 29 B4 24 ? ? ? ? 0F 29 BC 24 ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 4C 8B EA",
		//1.21.0.03
		"48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? B8 ? ? ? ? E8 ? ? ? ? 48 2B E0 0F 29 B4 24 ? ? ? ? 0F 29 BC 24 ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 48 8B F2 48 89 55",
		//1.21.20.24 preview
		"48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? B8 ? ? ? ? E8 ? ? ? ? 48 2B E0 0F 29 B4 24 ? ? ? ? 0F 29 BC 24 ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 48 8B F2 48 8B F9"
	)) {
		offsetToMaterialsManager = 816;
	} else {
		printf("Failed to hook mce::framebuilder::BgfxFrameBuilder::endFrame\n");
	}

	discardFrame = (PFN_mce_framebuilder_BgfxFrameBuilder_discardFrame)FindSignatures(
		//1.20.30.02
		"48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 44 24 ? 0F B6 EA 48 8B F1",
		//1.20.30.21 preview
		"48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 56 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 84 24 ? ? ? ? 0F B6 EA 48 8B F1 0F 57 C0"
	);
	if (!discardFrame) {
		printf("mce::framebuilder::BgfxFrameBuilder::discardFrame not found\n");
	}

	freeShaderBlobs = (PFN_dragon_materials_CompiledMaterialManager_freeShaderBlobs)FindSignatures(
		//1.20.1.02
		"48 89 74 24 ? 57 48 83 EC ? 48 8B F9 48 83 C1 ? FF 15 ? ? ? ? 85 C0"
	);
	if (!freeShaderBlobs) {
		printf("dragon::materials::CompiledMaterialManager::freeShaderBlobs not found\n");
	}

	Options::reloadShadersAvailable = offsetToMaterialsManager >= 0 && discardFrame && freeShaderBlobs;
}
