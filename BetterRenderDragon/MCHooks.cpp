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
	uintptr_t resourcePackManagerPtr = FindSignature(
		"48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 41 0F B6 F1 49 8B D8 4C 8B F2 48 8B F9"
	);
	if (!resourcePackManagerPtr) {
		//1.20.1.02
		resourcePackManagerPtr = FindSignature(
			"48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 41 0F B6 F1 49 8B F8 4C 8B F2 48 8B D9"
		);
	}
	if (!resourcePackManagerPtr) {
		//1.20.10.23 preview
		resourcePackManagerPtr = FindSignature(
			"48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 45 0F B6 F1 49 8B F8 48 8B F2 48 8B D9"
		);
	}
	if (!resourcePackManagerPtr) {
		//1.20.30.21 preview
		resourcePackManagerPtr = FindSignature(
			"4C 8B DC 53 55 56 57 41 56 48 81 EC ? ? ? ? 41 0F B6 E9 49 8B D8 48 8B F2 48 8B F9"
		);
	}
	if (resourcePackManagerPtr) {
		Hook(ResourcePackManager_constructor, (void*)resourcePackManagerPtr);
	} else {
		printf("Failed to hook ResourcePackManager::ResourcePackManager\n");
	}

	//1.19.40
	uintptr_t readFilePtr = FindSignature(
		"48 89 5C 24 ? 55 56 57 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 45 70 49 8B C0"
	);
	if (!readFilePtr) {
		//1.20.0.23 preview
		readFilePtr = FindSignature(
			"48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 45 20 49 8B C0"
		);
	}
	if (!readFilePtr) {
		//1.20.30.21 preview
		readFilePtr = FindSignature(
			"48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 49 8B C0 48 8B FA"
		);
	}
	if (!readFilePtr) {
		//1.20.30.02
		readFilePtr = FindSignature(
			"48 89 5C 24 ? 55 56 57 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 49 8B C0"
		);
	}
	
	if (readFilePtr) {
		Hook(readFile, (void*)readFilePtr);
	} else {
		printf("Failed to hook readFile\n");
	}
}
