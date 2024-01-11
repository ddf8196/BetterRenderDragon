#include <windows.h>
#include <psapi.h>
#include <atlbase.h>
#include <initguid.h>
#include <d3d12.h>
#include <d3d11.h>

#include <string>
#include <memory>

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/backends/imgui_impl_dx12.h"
#include "imgui/backends/imgui_impl_dx11.h"
#include "imgui_impl_winrt.h"

#include "ImGuiHooks.h"
#include "HookAPI.h"
#include "Options.h"
#include "Util.h"
#include "Version.h"

//=======================================================================================================================================================================

IDXGIFactory2* factory;
IUnknown* coreWindow;
bool imguiInitialized = false;
bool isChangingUIKey = false;
bool justChangedKey = false;

std::string cpuName;
std::string gpuName;
std::string rendererType;

//=======================================================================================================================================================================

//std::atomic_bool running = true;
//DWORD WINAPI trySaveOptions(LPVOID lpThreadParameter) {
//	while (running) {
//		if (Options::dirty) {
//			Options::save();
//			Options::dirty = false;
//		}
//		Sleep(1000);
//	}
//	return 0;
//}

//=======================================================================================================================================================================

void updateOptions() {
	static float saveTimer = 0.0f;

	static bool showImGui = Options::showImGui;
	static bool performanceEnabled = Options::performanceEnabled;
	static bool vanilla2DeferredEnabled = Options::vanilla2DeferredEnabled;
	static bool deferredRenderingEnabled = Options::deferredRenderingEnabled;
	static bool forceEnableDeferredTechnicalPreview = Options::forceEnableDeferredTechnicalPreview;
	static bool disableRendererContextD3D12RTX = Options::disableRendererContextD3D12RTX;
	static bool materialBinLoaderEnabled = Options::materialBinLoaderEnabled;
	static bool redirectShaders = Options::redirectShaders;
	static bool customUniformsEnabled = Options::customUniformsEnabled;
	static bool windowSettingsEnabled = Options::windowSettingsEnabled;

	if (showImGui != Options::showImGui
		|| performanceEnabled != Options::performanceEnabled
		|| vanilla2DeferredEnabled != Options::vanilla2DeferredEnabled
		|| deferredRenderingEnabled != Options::deferredRenderingEnabled
		|| forceEnableDeferredTechnicalPreview != Options::forceEnableDeferredTechnicalPreview
		|| disableRendererContextD3D12RTX != Options::disableRendererContextD3D12RTX
		|| materialBinLoaderEnabled != Options::materialBinLoaderEnabled
		|| redirectShaders != Options::redirectShaders
		|| customUniformsEnabled != Options::customUniformsEnabled
		|| windowSettingsEnabled != Options::windowSettingsEnabled) {

		Options::dirty = true;
		saveTimer = 3.0f;

		showImGui = Options::showImGui;
		performanceEnabled = Options::performanceEnabled;
		vanilla2DeferredEnabled = Options::vanilla2DeferredEnabled;
		deferredRenderingEnabled = Options::deferredRenderingEnabled;
		forceEnableDeferredTechnicalPreview = Options::forceEnableDeferredTechnicalPreview;
		disableRendererContextD3D12RTX = Options::disableRendererContextD3D12RTX;
		materialBinLoaderEnabled = Options::materialBinLoaderEnabled;
		redirectShaders = Options::redirectShaders;
		customUniformsEnabled = Options::customUniformsEnabled;
		windowSettingsEnabled = Options::windowSettingsEnabled;
	}

	//TODO: Put it on a separate thread
	if (saveTimer > 0.0f) {
		saveTimer -= ImGui::GetIO().DeltaTime;
		if (saveTimer <= 0.0f) {
			saveTimer = 0.0f;
			if (Options::dirty) {
				Options::save();
				Options::dirty = false;
			}
		}
	}
}


void updateKeys() {
	static bool prevToggleImGui = false;
	static bool prevToggleDeferredRendering = false;

	bool toggleImGui = ImGui::IsKeyPressed((ImGuiKey)Options::uikey) && !justChangedKey;
	if (!toggleImGui)
		justChangedKey = false;
	if (toggleImGui && !prevToggleImGui)
		Options::showImGui = !Options::showImGui;
	prevToggleImGui = toggleImGui;

	bool toggleDR = ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyDown(ImGuiKey_Semicolon);
	if (toggleDR && !prevToggleDeferredRendering)
		Options::deferredRenderingEnabled = !Options::deferredRenderingEnabled;
	prevToggleDeferredRendering = toggleDR;
}

void updateImGui() {
	//static bool showDemo = false;
	static bool showModuleManager = false;
	static bool showAbout = false;
	static bool showChangelog = false;

	bool resetLayout = false;
	bool moduleManagerRequestFocus = false;
	bool aboutRequestFocus = false;
	bool changelogRequestFocus = false;

	updateKeys();
	updateOptions();

	ImGui::NewFrame();
	if (Options::showImGui) {
		auto& io = ImGui::GetIO();

		ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(340, 275), ImGuiCond_FirstUseEver);
		ImGuiWindowFlags mainWindowFlags = ImGuiWindowFlags_MenuBar;

		if (ImGui::Begin("BetterRenderDragon", &Options::showImGui, mainWindowFlags)) {
			if (ImGui::BeginMenuBar()) {
				if (ImGui::BeginMenu("View")) {
					if (ImGui::MenuItem("Open Module Manager", NULL)) {
						showModuleManager = true;
						moduleManagerRequestFocus = true;
					}
					//if (ImGui::MenuItem("Open ImGui Demo Window", NULL)) {
					//	showDemo = true;
					//}
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Windows")) {
					if (ImGui::MenuItem("Reset window position and size", NULL))
						resetLayout = true;
					if (showModuleManager || showAbout/* || showDemo*/)
						ImGui::Separator();
					if (showModuleManager && ImGui::MenuItem("Module Manager", NULL))
						moduleManagerRequestFocus = true;
					if (showAbout && ImGui::MenuItem("About", NULL))
						aboutRequestFocus = true;
					if (showChangelog && ImGui::MenuItem("Changelog", NULL))
						changelogRequestFocus = true;
					//if (showDemo)
					//	ImGui::MenuItem("ImGui Demo Window", NULL);
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Help")) {
					if (ImGui::MenuItem("About", NULL)) {
						showAbout = true;
						aboutRequestFocus = true;
					}
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Changelog")) {
					if (ImGui::MenuItem("Changelog", NULL)) {
						showChangelog = true;
						changelogRequestFocus = true;
					}
					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}

			ImGui::Text("BetterRenderDragon %s", BetterRDVersion);
			ImGui::NewLine();

			if (Options::performanceEnabled && ImGui::CollapsingHeader("Performance", ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::Indent();
				ImGui::Text("FPS: %.01f", io.Framerate);
				ImGui::Text("Frame Time: %.01fms", io.DeltaTime * 1000.0f);
				ImGui::Unindent();
			}

			if (Options::vanilla2DeferredEnabled && ImGui::CollapsingHeader("Vanilla2Deferred", ImGuiTreeNodeFlags_DefaultOpen)) {
				if (!Options::vanilla2DeferredAvailable)
					ImGui::BeginDisabled();
				ImGui::Indent();
				if (Options::newVideoSettingsAvailable)
					ImGui::Checkbox("Force Enable Deferred Technical Preview", &Options::forceEnableDeferredTechnicalPreview);
				else
					ImGui::Checkbox("Enable Deferred Rendering", &Options::deferredRenderingEnabled);
				ImGui::Checkbox("Disable RTX (Requires restart)", &Options::disableRendererContextD3D12RTX);
				ImGui::Unindent();
				if (!Options::vanilla2DeferredAvailable)
					ImGui::EndDisabled();
			}

			if (Options::materialBinLoaderEnabled && ImGui::CollapsingHeader("MaterialBinLoader", ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::Indent();
				ImGui::Checkbox("Load shaders from resource pack", &Options::redirectShaders);
				ImGui::Unindent();
			}

#if 0
			if (Options::windowSettingsEnabled && ImGui::CollapsingHeader("Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::Indent();
				if (ImGui::Button(IsChangingUIKey == false ? "Set Open UI Key" : "Cancel")) {
					IsChangingUIKey = !IsChangingUIKey;
				}
				if (IsChangingUIKey) {
					for (int i = ImGuiKey_NamedKey_BEGIN; i < ImGuiKey_COUNT; i++) {
						if (ImGui::IsKeyDown((ImGuiKey)i) && !ImGui::IsMouseKey((ImGuiKey)i)) {
							Options::uikey = i;
							IsChangingUIKey = false;
							ImGui::GetIO().AddKeyEvent((ImGuiKey)Options::uikey, false);
							JustChangedKey = true;
							break;
						}
					}
				}
				ImGui::Text("UI Key: %s", ImGui::GetKeyName((ImGuiKey)Options::uikey));
				ImGui::Unindent();
			}
#endif

			if (Options::customUniformsEnabled && ImGui::CollapsingHeader("CustomUniforms", ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::Indent();
				ImGui::Unindent();
			}
			ImGui::NewLine();
		}
		ImGui::End();
		
		if (showModuleManager) {
			if (moduleManagerRequestFocus)
				ImGui::SetNextWindowFocus();
			ImGui::SetNextWindowSize(ImVec2(300, 130), ImGuiCond_FirstUseEver);
			if (ImGui::Begin("BetterRenderDragon - Module Manager", &showModuleManager)) {
				if (ImGui::BeginTable("modulesTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
					ImGui::TableSetupColumn("Module");
					ImGui::TableSetupColumn("Enabled");
					ImGui::TableHeadersRow();

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("Performance");
					ImGui::TableSetColumnIndex(1); ImGui::Checkbox("##1", &Options::performanceEnabled);

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("Vanilla2Deferred");
					ImGui::TableSetColumnIndex(1); ImGui::Checkbox("##2", &Options::vanilla2DeferredEnabled);

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("MaterialBinLoader");
					ImGui::TableSetColumnIndex(1); ImGui::Checkbox("##3", &Options::materialBinLoaderEnabled);

#if 0
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("Settings");
					ImGui::TableSetColumnIndex(1); ImGui::Checkbox("##4", &Options::windowSettingsEnabled);
#endif

					//ImGui::TableNextRow();
					//ImGui::TableSetColumnIndex(0); ImGui::Text("CustomUniforms");
					//ImGui::TableSetColumnIndex(1); ImGui::Checkbox("", &Options::customUniformsEnabled);

					ImGui::EndTable();
				}
			}
			ImGui::End();
		}

		if (showAbout) {
			if (aboutRequestFocus)
				ImGui::SetNextWindowFocus();
			if (ImGui::Begin("BetterRenderDragon - About", &showAbout)) {
				ImGui::Text("BetterRenderDragon %s", BetterRDVersion);
				ImGui::Text("https://github.com/ddf8196/BetterRenderDragon");
			}
			ImGui::End();
		}

		if (showChangelog) {
			if (changelogRequestFocus)
				ImGui::SetNextWindowFocus();
			if (ImGui::Begin("BetterRenderDragon - Changelog", &showChangelog)) {
				ImGui::Text("V1.3.3\n Support 1.20.1");
				ImGui::Text("V1.3.4\n Support 1.20.10\n No longer support version below 1.20.10 ");
				ImGui::Text("V1.3.5\n Support 1.20.32\n Fixed ImGui flickering on some devices\n Removed Vanilla2Deferred option");
				ImGui::Text("V1.3.6\n Support 1.20.40 and 1.20.50\n Added Force Enable Deferred Technical Preview option");
				ImGui::Text("V1.3.7\n Support 1.20.60.23 preview\n");
			}
			ImGui::End();
		}

		//if (showDemo)
		//	ImGui::ShowDemoWindow(&showDemo);
	}
	ImGui::EndFrame();

	if (resetLayout) {
		ImGui::ClearWindowSettings("BetterRenderDragon");
		ImGui::ClearWindowSettings("Dear ImGui Demo");
	}
}

void initializeImgui() {
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = nullptr;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
}

//=======================================================================================================================================================================

namespace ImGuiD3D12 {
	CComPtr<ID3D12Device> device;
	CComPtr<ID3D12DescriptorHeap> descriptorHeapBackBuffers;
	CComPtr<ID3D12DescriptorHeap> descriptorHeapImGuiRender;
	ID3D12CommandQueue* commandQueue;

	struct BackBufferContext {
		CComPtr<ID3D12CommandAllocator> commandAllocator;
		CComPtr<ID3D12GraphicsCommandList> commandList;
		ID3D12Resource* resource = nullptr;
		D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle = { 0 };
	};

	uint32_t backBufferCount = 0;
	std::unique_ptr<BackBufferContext[]> backBufferContext;

	void createRT(IDXGISwapChain* swapChain) {
		const auto rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = descriptorHeapBackBuffers->GetCPUDescriptorHandleForHeapStart();

		for (uint32_t i = 0; i < backBufferCount; i++) {
			ID3D12Resource* pBackBuffer = nullptr;
			backBufferContext[i].descriptorHandle = rtvHandle;
			swapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
			device->CreateRenderTargetView(pBackBuffer, nullptr, rtvHandle);
			backBufferContext[i].resource = pBackBuffer;
			rtvHandle.ptr += rtvDescriptorSize;
		}
	}

	void releaseRT() {
		for (size_t i = 0; i < backBufferCount; i++) {
			if (backBufferContext[i].resource) {
				backBufferContext[i].resource->Release();
				backBufferContext[i].resource = nullptr;
			}
		}
	}

	bool initializeImguiBackend(IDXGISwapChain* pSwapChain) {
		if (SUCCEEDED(pSwapChain->GetDevice(IID_ID3D12Device, (void**)&device))) {
			Options::vanilla2DeferredAvailable = true;

			initializeImgui();

			DXGI_SWAP_CHAIN_DESC desc;
			pSwapChain->GetDesc(&desc);

			backBufferCount = desc.BufferCount;
			backBufferContext.reset(new BackBufferContext[backBufferCount]);

			for (size_t i = 0; i < backBufferCount; i++) {
				if (device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&backBufferContext[i].commandAllocator)) != S_OK) {
					return false;
				}

				if (device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, backBufferContext[i].commandAllocator, NULL, IID_PPV_ARGS(&backBufferContext[i].commandList)) != S_OK ||
					backBufferContext[i].commandList->Close() != S_OK) {
					return false;
				}
			}

			D3D12_DESCRIPTOR_HEAP_DESC descriptorImGuiRender = {};
			descriptorImGuiRender.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			descriptorImGuiRender.NumDescriptors = backBufferCount;
			descriptorImGuiRender.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

			if (device->CreateDescriptorHeap(&descriptorImGuiRender, IID_PPV_ARGS(&descriptorHeapImGuiRender)) != S_OK) {
				return false;
			}

			D3D12_DESCRIPTOR_HEAP_DESC descriptorBackBuffers;
			descriptorBackBuffers.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			descriptorBackBuffers.NumDescriptors = backBufferCount;
			descriptorBackBuffers.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			descriptorBackBuffers.NodeMask = 1;

			if (device->CreateDescriptorHeap(&descriptorBackBuffers, IID_PPV_ARGS(&descriptorHeapBackBuffers)) != S_OK) {
				return false;
			}

			createRT(pSwapChain);

			ImGui_ImplWinRT_Init(coreWindow);
			ImGui_ImplDX12_Init(device, backBufferCount, DXGI_FORMAT_R8G8B8A8_UNORM, descriptorHeapImGuiRender, descriptorHeapImGuiRender->GetCPUDescriptorHandleForHeapStart(), descriptorHeapImGuiRender->GetGPUDescriptorHandleForHeapStart());
			ImGui_ImplDX12_CreateDeviceObjects();
		}
		return true;
	}

	void renderImGui(IDXGISwapChain3* swapChain) {
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWinRT_NewFrame();

		updateImGui();

		BackBufferContext& currentBufferContext = backBufferContext[swapChain->GetCurrentBackBufferIndex()];
		currentBufferContext.commandAllocator->Reset();

		D3D12_RESOURCE_BARRIER barrier;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = currentBufferContext.resource;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

		ID3D12DescriptorHeap* descriptorHeaps[1] = { descriptorHeapImGuiRender.p };
		currentBufferContext.commandList->Reset(currentBufferContext.commandAllocator, nullptr);
		currentBufferContext.commandList->ResourceBarrier(1, &barrier);
		currentBufferContext.commandList->OMSetRenderTargets(1, &currentBufferContext.descriptorHandle, FALSE, nullptr);
		currentBufferContext.commandList->SetDescriptorHeaps(1, descriptorHeaps);

		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), currentBufferContext.commandList);

		ID3D12CommandList* commandLists[1] = { currentBufferContext.commandList.p };
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		currentBufferContext.commandList->ResourceBarrier(1, &barrier);
		currentBufferContext.commandList->Close();
		commandQueue->ExecuteCommandLists(1, commandLists);
	}

	PFN_IDXGISwapChain_Present Original_IDXGISwapChain_Present = nullptr;
	HRESULT STDMETHODCALLTYPE IDXGISwapChain_Present_Hook(IDXGISwapChain* This, UINT SyncInterval, UINT Flags) {
		CComPtr<IDXGISwapChain3> swapChain3;
		if (FAILED(This->QueryInterface<IDXGISwapChain3>(&swapChain3))) {
			return Original_IDXGISwapChain_Present(This, SyncInterval, Flags);
		}

		if (!imguiInitialized) {
			printf("Initializing ImGui on Direct3D 12\n");
			if (initializeImguiBackend(This)) {
				imguiInitialized = true;
			} else {
				printf("ImGui is not initialized\n");
				return Original_IDXGISwapChain_Present(This, SyncInterval, Flags);
			}
		}

		renderImGui(swapChain3);

		return Original_IDXGISwapChain_Present(This, SyncInterval, Flags);
	}

	PFN_IDXGISwapChain_ResizeBuffers Original_IDXGISwapChain_ResizeBuffers;
	HRESULT STDMETHODCALLTYPE IDXGISwapChain_ResizeBuffers_Hook(IDXGISwapChain* This, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags) {
		releaseRT();
		HRESULT hResult = Original_IDXGISwapChain_ResizeBuffers(This, BufferCount, Width, Height, NewFormat, SwapChainFlags);
		createRT(This);
		return hResult;
	}
}

//=======================================================================================================================================================================

namespace ImGuiD3D11 {
	ID3D11Device* device;
	CComPtr<ID3D11DeviceContext> deviceContext;

	uint32_t backBufferCount = 0;
	ID3D11RenderTargetView** renderTargetViews;

	void createRT(IDXGISwapChain* swapChain) {
		for (uint32_t i = 0; i < backBufferCount; i++) {
			CComPtr<ID3D11Resource> backBuffer;
			swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer));

			ID3D11RenderTargetView* rtv;
			device->CreateRenderTargetView(backBuffer, nullptr, &rtv);

			renderTargetViews[i] = rtv;
		}
	}

	void releaseRT() {
		for (size_t i = 0; i < backBufferCount; i++) {
			if (renderTargetViews[i]) {
				renderTargetViews[i]->Release();
				renderTargetViews[i] = nullptr;
			}
		}
	}

	bool initializeImguiBackend(IDXGISwapChain* pSwapChain) {
		Options::vanilla2DeferredAvailable = false;

		initializeImgui();

		DXGI_SWAP_CHAIN_DESC desc;
		pSwapChain->GetDesc(&desc);

		backBufferCount = desc.BufferCount;
		renderTargetViews = new ID3D11RenderTargetView*[backBufferCount];

		device->GetImmediateContext(&deviceContext);

		createRT(pSwapChain);

		ImGui_ImplWinRT_Init(coreWindow);
		ImGui_ImplDX11_Init(device, deviceContext);
		ImGui_ImplDX11_CreateDeviceObjects();

		return true;
	}

	void renderImGui(IDXGISwapChain3* swapChain) {
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWinRT_NewFrame();

		updateImGui();

		ID3D11RenderTargetView* currentRTV = renderTargetViews[swapChain->GetCurrentBackBufferIndex()];
		deviceContext->OMSetRenderTargets(1, &currentRTV, nullptr);

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}

	PFN_IDXGISwapChain_Present Original_IDXGISwapChain_Present = nullptr;
	HRESULT STDMETHODCALLTYPE IDXGISwapChain_Present_Hook(IDXGISwapChain* This, UINT SyncInterval, UINT Flags) {
		CComPtr<IDXGISwapChain3> swapChain3;
		if (FAILED(This->QueryInterface<IDXGISwapChain3>(&swapChain3))) {
			return Original_IDXGISwapChain_Present(This, SyncInterval, Flags);
		}

		if (imguiInitialized) {
			renderImGui(swapChain3);
		}

		return Original_IDXGISwapChain_Present(This, SyncInterval, Flags);
	}

	PFN_IDXGISwapChain_ResizeBuffers Original_IDXGISwapChain_ResizeBuffers;
	HRESULT STDMETHODCALLTYPE IDXGISwapChain_ResizeBuffers_Hook(IDXGISwapChain* This, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags) {
		releaseRT();
		HRESULT hResult = Original_IDXGISwapChain_ResizeBuffers(This, BufferCount, Width, Height, NewFormat, SwapChainFlags);
		createRT(This);
		return hResult;
	}
}

//=======================================================================================================================================================================

PFN_IDXGIFactory2_CreateSwapChainForCoreWindow Original_IDXGIFactory2_CreateSwapChainForCoreWindow;
HRESULT STDMETHODCALLTYPE IDXGIFactory2_CreateSwapChainForCoreWindow_Hook(IDXGIFactory2* This, IUnknown* pDevice, IUnknown* pWindow, const DXGI_SWAP_CHAIN_DESC1* pDesc, IDXGIOutput* pRestrictToOutput, IDXGISwapChain1** ppSwapChain) {
	if (pWindow)
		coreWindow = pWindow;

	HRESULT hResult = Original_IDXGIFactory2_CreateSwapChainForCoreWindow(This, pDevice, pWindow, pDesc, pRestrictToOutput, ppSwapChain);
	if (SUCCEEDED(hResult)) {
		IDXGISwapChain1* swapChain = *ppSwapChain;
		CComPtr<ID3D12CommandQueue> d3d12CommandQueue;
		CComPtr<ID3D11Device> d3d11Device;
		if (SUCCEEDED(pDevice->QueryInterface(&d3d12CommandQueue))) {
			//Direct3D 12
			ImGuiD3D12::commandQueue = (ID3D12CommandQueue*)pDevice;
			if (!ImGuiD3D12::Original_IDXGISwapChain_Present)
				ReplaceVtable(*(void**)swapChain, 8, (void**)&ImGuiD3D12::Original_IDXGISwapChain_Present, ImGuiD3D12::IDXGISwapChain_Present_Hook);
			if (!ImGuiD3D12::Original_IDXGISwapChain_ResizeBuffers)
				ReplaceVtable(*(void**)swapChain, 13, (void**)&ImGuiD3D12::Original_IDXGISwapChain_ResizeBuffers, ImGuiD3D12::IDXGISwapChain_ResizeBuffers_Hook);
			//When the graphics API used by RenderDragon is D3D12, this function will be called in a non-main thread and later IDXGISwapChain::Present will be called three times in the main thread, so initialize ImGui later in IDXGISwapChain::Present
		} else if (SUCCEEDED(pDevice->QueryInterface(&d3d11Device))) {
			//Direct3D 11
			ImGuiD3D11::device = (ID3D11Device*)pDevice;
			if (!ImGuiD3D11::Original_IDXGISwapChain_Present)
				ReplaceVtable(*(void**)swapChain, 8, (void**)&ImGuiD3D11::Original_IDXGISwapChain_Present, ImGuiD3D11::IDXGISwapChain_Present_Hook);
			if (!ImGuiD3D11::Original_IDXGISwapChain_ResizeBuffers)
				ReplaceVtable(*(void**)swapChain, 13, (void**)&ImGuiD3D11::Original_IDXGISwapChain_ResizeBuffers, ImGuiD3D11::IDXGISwapChain_ResizeBuffers_Hook);
			// When the graphics API used by RenderDragon is D3D11, this function will be called in the main thread, and IDXGISwapChain::Present will all be called in a non-main thread, so initialize ImGui here immediately
			printf("Initializing ImGui on Direct3D 11\n");
			if (!(imguiInitialized = ImGuiD3D11::initializeImguiBackend(swapChain))) {
				printf("Failed to initialize ImGui on Direct3D 11\n");
			}
		} else {
			
		}
	}
	return hResult;
}

//=======================================================================================================================================================================

DeclareHook(CreateDXGIFactory1, HRESULT, REFIID riid, void** ppFactory) {
	HRESULT hResult = original(riid, ppFactory);
	if (IsEqualIID(IID_IDXGIFactory2, riid) && SUCCEEDED(hResult)) {
		printf("CreateDXGIFactory1 riid=IID_IDXGIFactory2\n");
		IDXGIFactory2* factory = (IDXGIFactory2*)*ppFactory;
		if (!Original_IDXGIFactory2_CreateSwapChainForCoreWindow)
			ReplaceVtable(*(void**)factory, 16, (void**)&Original_IDXGIFactory2_CreateSwapChainForCoreWindow, IDXGIFactory2_CreateSwapChainForCoreWindow_Hook);

		factory = factory;
	}
	return hResult;
}

//=======================================================================================================================================================================

void ImGuiHooks_Init() {
	printf("%s\n", __FUNCTION__);

	HMODULE dxgiModule = GetModuleHandleA("dxgi.dll");
	if (!dxgiModule) {
		return;
	}

	FARPROC CreateDXGIFactory1 = GetProcAddress(dxgiModule, "CreateDXGIFactory1");
	if (!CreateDXGIFactory1)
		return;

	Hook(CreateDXGIFactory1, CreateDXGIFactory1);
}

//=======================================================================================================================================================================
