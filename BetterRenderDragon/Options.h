#pragma once
#include <string>
#include <atomic>

class IOption {
public:
	virtual void record() = 0;
	virtual bool isChanged() = 0;
};

template<typename T>
class Option : public IOption {
public:
	Option() {
	}
	Option(const T& value) : value(value), prevValue(value) {
	}
	virtual void record() override {
		prevValue = value;
	}
	virtual bool isChanged() override {
		return value != prevValue;
	}
	T& get() {
		return value;
	}
	const T& get() const {
		return value;
	}
	void set(const T& value) {
		this->value = value;
	}
	T* ptr() {
		return &this->value;
	}
	const T* ptr() const {
		return &this->value;
	}
	operator T() {
		return value;
	}
	Option<T>& operator =(const T& value) {
		this->value = value;
		return *this;
	}
private:
	T value;
	T prevValue;
};

namespace Options {
	extern Option<bool> showImGui;

	extern Option<bool> performanceEnabled;

	extern Option<bool> windowSettingsEnabled;

	extern bool vanilla2DeferredAvailable;
	extern bool newVideoSettingsAvailable;
	extern Option<bool> vanilla2DeferredEnabled;
	extern Option<bool> deferredRenderingEnabled;
	extern Option<bool> forceEnableDeferredTechnicalPreview;
	extern Option<bool> disableRendererContextD3D12RTX;

	extern Option<bool> materialBinLoaderEnabled;
	extern Option<bool> redirectShaders;
	extern bool reloadShadersAvailable;
	extern std::atomic_bool reloadShaders;

	extern Option<bool> customUniformsEnabled;

	extern Option<int> uiKey;
	extern Option<int> reloadShadersKey;

	extern std::string optionsDir;
	extern std::string optionsFile;

	extern bool init();
	extern bool load();
	extern bool save();
	extern void record();
	extern bool isDirty();
};
