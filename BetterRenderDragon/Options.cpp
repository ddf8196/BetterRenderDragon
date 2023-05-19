#include <string>
#include <fstream>
#include <filesystem>

#include <Windows.Storage.h>
#include <wrl.h>

#include "nlohmann/json.hpp"
#include "Options.h"
#include "Util.h"

bool Options::showImGui = true;

bool Options::performanceEnabled = true;

bool Options::vanilla2DeferredEnabled = true;
bool Options::deferredRenderingEnabled = false;
bool Options::limitShaderModel = true;
bool Options::disableRendererContextD3D12RTX = false;

bool Options::materialBinLoaderEnabled = true;
bool Options::redirectShaders = true;

bool Options::customUniformsEnabled = false;

std::atomic_bool Options::dirty = false;

std::string Options::optionsDir;
std::string Options::optionsFile;

using nlohmann::json;

using ABI::Windows::Storage::IApplicationDataStatics;
using ABI::Windows::Storage::IApplicationData;
using ABI::Windows::Storage::IStorageFolder;
using ABI::Windows::Storage::IStorageItem;
using Microsoft::WRL::ComPtr;
using Microsoft::WRL::Wrappers::HString;
using Microsoft::WRL::Wrappers::HStringReference;

std::string getLocalStatePath() {
	HRESULT hResult;

	ComPtr<IApplicationDataStatics> applicationDataStatics;
	hResult = RoGetActivationFactory(HStringReference(RuntimeClass_Windows_Storage_ApplicationData).Get(), IID_PPV_ARGS(&applicationDataStatics));
	if (FAILED(hResult)) {
		printf("%s %d hr=0x%08X\n", __FUNCTION__, __LINE__, hResult);
		return "";
	}
		

	ComPtr<IApplicationData> applicationData;
	hResult = applicationDataStatics->get_Current(&applicationData);
	if (FAILED(hResult)) {
		printf("%s %d hr=0x%08X\n", __FUNCTION__, __LINE__, hResult);
		return "";
	}

	ComPtr<IStorageFolder> localState;
	hResult = applicationData->get_LocalFolder(&localState);
	if (FAILED(hResult)) {
		printf("%s %d hr=0x%08X\n", __FUNCTION__, __LINE__, hResult);
		return "";
	}

	ComPtr<IStorageItem> localStateStorageItem;
	hResult = localState.As(&localStateStorageItem);
	if (FAILED(hResult)) {
		printf("%s %d hr=0x%08X\n", __FUNCTION__, __LINE__, hResult);
		return "";
	}

	HString localStatePath;
	hResult = localStateStorageItem->get_Path(localStatePath.GetAddressOf());
	if (FAILED(hResult)) {
		printf("%s %d hr=0x%08X\n", __FUNCTION__, __LINE__, hResult);
		return "";
	}

	uint32_t length;
	const wchar_t* pathRawBuffer = localStatePath.GetRawBuffer(&length);
	std::wstring pathWstr(pathRawBuffer, length);

	return wstringToString(pathWstr);
}

bool Options::init() {
	if (optionsDir.empty()) {
		std::string localStatePath = getLocalStatePath();
		printf("%s\n", localStatePath.c_str());
		if (localStatePath.empty()) {
			return false;
		}
		//optionsDir = localStatePath + "\\BetterRenderDragon";
		optionsDir = localStatePath;
		optionsFile = optionsDir + "\\BetterRenderDragon.json";
	}
	//if (!std::filesystem::exists(optionsDir)) {
	//	if (!std::filesystem::create_directories(optionsDir)) {
	//		return false;
	//	}
	//}
	//if (!std::filesystem::is_directory(optionsDir)) {
	//	return false;
	//}
	return true;
}

bool Options::load() {
	if (!std::filesystem::exists(optionsFile)) {
		return save();
	}

	if (!std::filesystem::is_regular_file(optionsFile)) {
		return false;
	}

	json data;
	try {
		std::ifstream ifs(optionsFile, std::ifstream::binary);
		ifs >> data;
	} catch (json::parse_error& e) {
		printf("Failed to parse json: %s", e.what());
		return false;
	}

	showImGui = data["showImGui"];

	performanceEnabled = data["performanceEnabled"];

	vanilla2DeferredEnabled = data["vanilla2DeferredEnabled"];
	deferredRenderingEnabled = data["deferredRenderingEnabled"];
	limitShaderModel = data["limitShaderModel"];
	disableRendererContextD3D12RTX = data["disableRendererContextD3D12RTX"];

	materialBinLoaderEnabled = data["materialBinLoaderEnabled"];
	redirectShaders = data["redirectShaders"];

	//customUniformsEnabled = data["customUniformsEnabled"];

	return true;
}

bool Options::save() {
	json data;
	data["showImGui"] = showImGui;

	data["performanceEnabled"] = performanceEnabled;

	data["vanilla2DeferredEnabled"] = vanilla2DeferredEnabled;
	data["deferredRenderingEnabled"] = deferredRenderingEnabled;
	data["limitShaderModel"] = limitShaderModel;
	data["disableRendererContextD3D12RTX"] = disableRendererContextD3D12RTX;

	data["materialBinLoaderEnabled"] = materialBinLoaderEnabled;
	data["redirectShaders"] = redirectShaders;

	//data["customUniformsEnabled"] = customUniformsEnabled;

	std::ofstream ofs(optionsFile, std::ifstream::binary);
	ofs << std::setw(4) << data << std::endl;
	return true;
}