#pragma once
#include <cstdint>
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
	ResourceFileSystem mFileSystem; //this+0x0
	Core::PathBuffer<std::string> mPath; //this+0x8
	uint64_t mPathHash; //this+0x28
	uint64_t mFullHash; //this+0x30

	ResourceLocation(Core::Path& path) : mPath(path.mPathPart.mUtf8StdString.c_str()) {
		mFileSystem = ResourceFileSystem::UserPackage;
		_computeHashes();
	}

	ResourceLocation(Core::Path&& path) : mPath(path.mPathPart.mUtf8StdString.c_str()) {
		mFileSystem = ResourceFileSystem::UserPackage;
		_computeHashes();
	}

private:
	void _computeHashes() {
		const char* mPath; // rdx
		uint8_t v3; // cl
		uint8_t v4; // rax
		uint8_t v5; // rcx

		mPath = this->mPath.mContainer.c_str();
		if (mPath && (v3 = *mPath) != 0)
		{
			v4 = 0xCBF29CE484222325ui64;
			do
			{
				mPath = mPath + 1;
				v4 = v3 ^ (0x100000001B3i64 * v4);
				v3 = *(uint8_t*)mPath;
			} while (*(uint8_t*)mPath);
		}
		else
		{
			v4 = 0i64;
		}
		v5 = (uint8_t)this->mFileSystem ^ 0xCBF29CE484222325ui64;
		this->mPathHash = v4;
		this->mFullHash = v4 ^ (0x100000001B3i64 * v5);
	}
};