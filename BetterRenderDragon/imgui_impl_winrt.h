#pragma once
#include <windows.h>
#include <wrl.h>
#include <Windows.UI.Core.h>

#include <imgui/imgui.h>

bool ImGui_ImplWinRT_Init(IUnknown* CoreWindow);
void ImGui_ImplWinRT_Shutdown();
void ImGui_ImplWinRT_NewFrame();

class ImGuiInputEventHandler {
public:
    ImGuiInputEventHandler(ABI::Windows::UI::Core::ICoreWindow* window);
	~ImGuiInputEventHandler();
private:
	Microsoft::WRL::ComPtr<ABI::Windows::UI::Core::ICoreWindow> window;

	EventRegistrationToken pointerMovedToken, pointerExitedToken, pointerPressedToken, pointerReleasedToken, pointerWheelChangedToken;
    EventRegistrationToken keyDownToken, keyUpToken;
    EventRegistrationToken characterReceivedToken;

    void UpdateMouseButtonState(ABI::Windows::UI::Core::IPointerEventArgs* args);
	HRESULT OnPointerMoved(ABI::Windows::UI::Core::ICoreWindow* sender, ABI::Windows::UI::Core::IPointerEventArgs* args);
	HRESULT OnPointerExited(ABI::Windows::UI::Core::ICoreWindow* sender, ABI::Windows::UI::Core::IPointerEventArgs* args);
	HRESULT OnPointerPressed(ABI::Windows::UI::Core::ICoreWindow* sender, ABI::Windows::UI::Core::IPointerEventArgs* args);
	HRESULT OnPointerReleased(ABI::Windows::UI::Core::ICoreWindow* sender, ABI::Windows::UI::Core::IPointerEventArgs* args);
	HRESULT OnPointerWheelChanged(ABI::Windows::UI::Core::ICoreWindow* sender, ABI::Windows::UI::Core::IPointerEventArgs* args);
	HRESULT OnKeyDown(ABI::Windows::UI::Core::ICoreWindow* sender, ABI::Windows::UI::Core::IKeyEventArgs* args);
	HRESULT OnKeyUp(ABI::Windows::UI::Core::ICoreWindow* sender, ABI::Windows::UI::Core::IKeyEventArgs* args);
	HRESULT OnCharacterReceived(ABI::Windows::UI::Core::ICoreWindow* sender, ABI::Windows::UI::Core::ICharacterReceivedEventArgs* args);
};

