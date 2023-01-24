#pragma once
#include <d3d12.h>

void D3D12Hooks_Init();

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------

/*13*/ typedef HRESULT (STDMETHODCALLTYPE* PFN_ID3D12Device_CheckFeatureSupport)(ID3D12Device*, D3D12_FEATURE, void*, UINT);
/*18*/ typedef void (STDMETHODCALLTYPE* PFN_ID3D12Device_CreateShaderResourceView)(ID3D12Device*, ID3D12Resource*, const D3D12_SHADER_RESOURCE_VIEW_DESC*, D3D12_CPU_DESCRIPTOR_HANDLE);
/*27*/ typedef HRESULT(*PFN_ID3D12Device_CreateCommittedResource)(ID3D12Device*, const D3D12_HEAP_PROPERTIES*, D3D12_HEAP_FLAGS, const D3D12_RESOURCE_DESC*, D3D12_RESOURCE_STATES, const D3D12_CLEAR_VALUE*, REFIID, void**);
/*29*/ typedef HRESULT(STDMETHODCALLTYPE* PFN_ID3D12Device_CreatePlacedResource)(ID3D12Device*, ID3D12Heap*, UINT64, const D3D12_RESOURCE_DESC*, D3D12_RESOURCE_STATES, const D3D12_CLEAR_VALUE*, REFIID, void**);

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------

extern PFN_D3D12_CREATE_DEVICE Original_D3D12CreateDevice;

extern PFN_ID3D12Device_CheckFeatureSupport Original_ID3D12Device_CheckFeatureSupport;
extern PFN_ID3D12Device_CreateShaderResourceView Original_ID3D12Device_CreateShaderResourceView;
extern PFN_ID3D12Device_CreateCommittedResource Original_ID3D12Device_CreateCommittedResource;
extern PFN_ID3D12Device_CreatePlacedResource Original_ID3D12Device_CreatePlacedResource;

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------

HRESULT D3D12CreateDeviceHook(_In_opt_ IUnknown* pAdapter, D3D_FEATURE_LEVEL MinimumFeatureLevel, _In_ REFIID riid, _COM_Outptr_opt_ void** ppDevice);

HRESULT STDMETHODCALLTYPE ID3D12Device_CheckFeatureSupportHook(ID3D12Device* self, D3D12_FEATURE Feature, void* pFeatureSupportData, UINT FeatureSupportDataSize);
void STDMETHODCALLTYPE ID3D12Device_CreateShaderResourceViewHook(ID3D12Device* self, ID3D12Resource* pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);
HRESULT STDMETHODCALLTYPE ID3D12Device_CreateCommittedResourceHook(ID3D12Device* self, const D3D12_HEAP_PROPERTIES* pHeapProperties, D3D12_HEAP_FLAGS HeapFlags, const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, REFIID riidResource, void** ppvResource);
HRESULT STDMETHODCALLTYPE ID3D12Device_CreatePlacedResourceHook(ID3D12Device* self, ID3D12Heap* pHeap, UINT64 HeapOffset, const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, REFIID riid, void** ppvResource);
