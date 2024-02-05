#pragma once
#include <cstdint>
#include <functional>
#include "../File/Path.h"

enum class ResourceFileSystem {
	UserPackage = 0x0,
	AppPackage = 0x1,
	Raw = 0x2,
	RawPersistent = 0x3,
	SettingsDir = 0x4,
	ExternalDir = 0x5,
	ServerPackage = 0x6,
	DataDir = 0x7,
	UserDir = 0x8,
	ScreenshotsDir = 0x9,
	StoreCache = 0xA,
	Invalid = 0xB
};

class ResourceLocation {
public:
	ResourceLocation(const Core::Path& path) : mPath(path){
		mFileSystem = ResourceFileSystem::UserPackage;
		_computeHashes();
	}

	ResourceLocation(const Core::Path& path, ResourceFileSystem fileSystem) : mPath(path) {
		mFileSystem = fileSystem;
		_computeHashes();
	}

private:
	ResourceFileSystem mFileSystem; //this+0x0
	Core::HeapPathBuffer mPath; //this+0x8
	uint64_t mPathHash; //this+0x28
	size_t mFullHash; //this+0x30

	void _computeHashes() {
		uint64_t pathHash;
		if (!mPath.empty()) {
			pathHash = 0xCBF29CE484222325LL;
			for (auto c : this->mPath.getContainer()) {
				pathHash = c ^ (0x100000001B3LL * pathHash);
			}
		} else {
			pathHash = 0;
		}
		this->mPathHash = pathHash;
		this->mFullHash = pathHash ^ std::hash<ResourceFileSystem>{}(this->mFileSystem);
	}
};