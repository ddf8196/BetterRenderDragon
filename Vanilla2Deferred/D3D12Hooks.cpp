#include <unordered_map>

#include <initguid.h>
#include <d3d12.h>

#include "HookAPI.h"
#include "D3D12Hooks.h"

HMODULE d3d12dll = nullptr;
FARPROC D3D12CreateDeviceAddress = nullptr;

void D3D12Hooks_Init() {
	printf("%s\n", __FUNCTION__);

	d3d12dll = GetModuleHandleA("d3d12.dll");
	if (d3d12dll == nullptr) {
		printf("d3d12dll == nullptr\n");
		return;
	}
	D3D12CreateDeviceAddress = GetProcAddress(d3d12dll, "D3D12CreateDevice");
	HookFunction(D3D12CreateDeviceAddress, (void**)&Original_D3D12CreateDevice, D3D12CreateDeviceHook);
}

PFN_D3D12_CREATE_DEVICE Original_D3D12CreateDevice;
HRESULT D3D12CreateDeviceHook(_In_opt_ IUnknown* pAdapter, D3D_FEATURE_LEVEL MinimumFeatureLevel, _In_ REFIID riid, _COM_Outptr_opt_ void** ppDevice) {
	int hResult = Original_D3D12CreateDevice(pAdapter, MinimumFeatureLevel, riid, ppDevice);

	printf("D3D12CreateDevice MinimumFeatureLevel=0x%04X hResult=0x%08X\n", MinimumFeatureLevel, hResult);

	if (FAILED(hResult) || ppDevice == nullptr || !IsEqualIID(riid, IID_ID3D12Device)) {
		return hResult;
	}

	ID3D12Device* device = *(ID3D12Device**)ppDevice;

	bool hook = true;
	D3D12_FEATURE_DATA_D3D12_OPTIONS options;
	D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5;
	memset(&options, 0, sizeof(options));
	memset(&options5, 0, sizeof(options5));
	if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options))) || FAILED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(options5)))) {
		hook = false;
	}
	if (options.ResourceBindingTier < D3D12_RESOURCE_BINDING_TIER_2 || options5.RaytracingTier >= 11) {
		hook = false;
	}

	printf("ResourceBindingTier=%d\n", options.ResourceBindingTier);
	printf("RaytracingTier=%d\n", options5.RaytracingTier);

	if (hook) {
		ReplaceVtable(*(void**)device, 13, (void**)&Original_ID3D12Device_CheckFeatureSupport, ID3D12Device_CheckFeatureSupportHook);
		ReplaceVtable(*(void**)device, 18, (void**)&Original_ID3D12Device_CreateShaderResourceView, ID3D12Device_CreateShaderResourceViewHook);
		ReplaceVtable(*(void**)device, 27, (void**)&Original_ID3D12Device_CreateCommittedResource, ID3D12Device_CreateCommittedResourceHook);
		ReplaceVtable(*(void**)device, 29, (void**)&Original_ID3D12Device_CreatePlacedResource, ID3D12Device_CreatePlacedResourceHook);
	} else {
		UnhookFunction(Original_D3D12CreateDevice, D3D12CreateDeviceHook);
	}
	return hResult;
}

PFN_ID3D12Device_CheckFeatureSupport Original_ID3D12Device_CheckFeatureSupport;
HRESULT STDMETHODCALLTYPE ID3D12Device_CheckFeatureSupportHook(ID3D12Device* self, D3D12_FEATURE Feature, void* pFeatureSupportData, unsigned int FeatureSupportDataSize) {
	int hResult = Original_ID3D12Device_CheckFeatureSupport(self, Feature, pFeatureSupportData, FeatureSupportDataSize);
	if (Feature == D3D12_FEATURE_D3D12_OPTIONS5) {
		D3D12_FEATURE_DATA_D3D12_OPTIONS5* options5 = (D3D12_FEATURE_DATA_D3D12_OPTIONS5*)pFeatureSupportData;
		options5->RaytracingTier = (D3D12_RAYTRACING_TIER)11; //D3D12_RAYTRACING_TIER_1_1
	}

	if (Feature == D3D12_FEATURE_SHADER_MODEL) {
		D3D12_FEATURE_DATA_SHADER_MODEL* supportedShaderModel = (D3D12_FEATURE_DATA_SHADER_MODEL*)pFeatureSupportData;
		printf("HighestShaderModel=%d\n", supportedShaderModel->HighestShaderModel);

		if (supportedShaderModel->HighestShaderModel > D3D_SHADER_MODEL_5_1) {
			supportedShaderModel->HighestShaderModel = D3D_SHADER_MODEL_5_1;
		}
	}

	return hResult;
}

PFN_ID3D12Device_CreateShaderResourceView Original_ID3D12Device_CreateShaderResourceView;
void STDMETHODCALLTYPE ID3D12Device_CreateShaderResourceViewHook(ID3D12Device* self, ID3D12Resource* pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor) {
	if (pDesc->ViewDimension != D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE) {
		Original_ID3D12Device_CreateShaderResourceView(self, pResource, pDesc, DestDescriptor);
	} else {
		printf("%s ViewDimension=%d\n", __FUNCTION__, pDesc->ViewDimension);
		D3D12_SHADER_RESOURCE_VIEW_DESC* desc = (D3D12_SHADER_RESOURCE_VIEW_DESC*)pDesc;
		desc->ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		desc->Buffer.FirstElement = 0;
		desc->Buffer.NumElements = 1;
		desc->Buffer.StructureByteStride = 1;
		desc->Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		Original_ID3D12Device_CreateShaderResourceView(self, pResource, desc, DestDescriptor);
	}
}

PFN_ID3D12Device_CreateCommittedResource Original_ID3D12Device_CreateCommittedResource;
HRESULT ID3D12Device_CreateCommittedResourceHook(ID3D12Device* self, const D3D12_HEAP_PROPERTIES* pHeapProperties, D3D12_HEAP_FLAGS HeapFlags, const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, REFIID riidResource, void** ppvResource) {
	if (InitialResourceState == D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE) {
		printf("%s InitialResourceState=%d\n", __FUNCTION__, InitialResourceState);
		InitialResourceState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	}
	return Original_ID3D12Device_CreateCommittedResource(self, pHeapProperties, HeapFlags, pDesc, InitialResourceState, pOptimizedClearValue, riidResource, ppvResource);;
}

PFN_ID3D12Device_CreatePlacedResource Original_ID3D12Device_CreatePlacedResource;
HRESULT ID3D12Device_CreatePlacedResourceHook(ID3D12Device* self, ID3D12Heap* pHeap, UINT64 HeapOffset, const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, REFIID riid, void** ppvResource) {
	if (InitialState == D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE) {
		printf("%s InitialState=%d\n", __FUNCTION__, InitialState);
		InitialState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	}
	return Original_ID3D12Device_CreatePlacedResource(self, pHeap, HeapOffset, pDesc, InitialState, pOptimizedClearValue, riid, ppvResource);
}
