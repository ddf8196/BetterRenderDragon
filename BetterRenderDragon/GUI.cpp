#include "GUI.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include "Options.h"
#include "Version.h"

void initializeImGui() {
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = nullptr;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
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
		if (ImGui::Begin("BetterRenderDragon", Options::showImGui.ptr(), ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_AlwaysAutoResize)) {
			if (ImGui::BeginMenuBar()) {
				if (ImGui::BeginMenu("View")) {
					if (ImGui::MenuItem("Open Module Manager")) {
						showModuleManager = true;
						moduleManagerRequestFocus = true;
					}
					//if (ImGui::MenuItem("Open ImGui Demo Window")) {
					//	showDemo = true;
					//}
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Windows")) {
					if (ImGui::MenuItem("Reset window position and size"))
						resetLayout = true;
					if (showModuleManager || showAbout/* || showDemo*/)
						ImGui::Separator();
					if (showModuleManager && ImGui::MenuItem("Module Manager"))
						moduleManagerRequestFocus = true;
					if (showAbout && ImGui::MenuItem("About"))
						aboutRequestFocus = true;
					if (showChangelog && ImGui::MenuItem("Changelog"))
						changelogRequestFocus = true;
					//if (showDemo)
					//	ImGui::MenuItem("ImGui Demo Window");
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Help")) {
					if (ImGui::MenuItem("About")) {
						showAbout = true;
						aboutRequestFocus = true;
					}
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Changelog")) {
					if (ImGui::MenuItem("Changelog")) {
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
				static float framerate = io.Framerate;
				static float deltaTime = io.DeltaTime;
				static float updateTimer = 0.5f;
				updateTimer -= io.DeltaTime;
				if (updateTimer <= 0) {
					framerate = io.Framerate;
					deltaTime = io.DeltaTime;
					updateTimer = 0.5f;
				}
				ImGui::Indent();
				ImGui::Text("FPS: %.01f", framerate);
				ImGui::Text("Frame Time: %.01fms", deltaTime * 1000.0f);
				ImGui::Unindent();
			}

			if (Options::vanilla2DeferredEnabled && ImGui::CollapsingHeader("Vanilla2Deferred", ImGuiTreeNodeFlags_DefaultOpen)) {
				if (!Options::vanilla2DeferredAvailable)
					ImGui::BeginDisabled();
				ImGui::Indent();
				if (Options::newVideoSettingsAvailable)
					ImGui::Checkbox("Force Enable Deferred Technical Preview", Options::forceEnableDeferredTechnicalPreview.ptr());
				else
					ImGui::Checkbox("Enable Deferred Rendering", Options::deferredRenderingEnabled.ptr());
				ImGui::Checkbox("Disable RTX (Requires restart)", Options::disableRendererContextD3D12RTX.ptr());
				ImGui::Unindent();
				if (!Options::vanilla2DeferredAvailable)
					ImGui::EndDisabled();
			}

			if (Options::materialBinLoaderEnabled && ImGui::CollapsingHeader("MaterialBinLoader", ImGuiTreeNodeFlags_DefaultOpen)) {
				ImGui::Indent();
				ImGui::Checkbox("Load shaders from resource pack", Options::redirectShaders.ptr());
				if (Options::reloadShadersAvailable) {
					bool disable = Options::reloadShaders;
					if (disable)
						ImGui::BeginDisabled();
					if (ImGui::Button("Reload shaders")) {
						Options::reloadShaders = true;
					}
					if (disable)
						ImGui::EndDisabled();
				}
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
			if (ImGui::Begin("BetterRenderDragon - Module Manager", &showModuleManager)) {
				if (ImGui::BeginTable("modulesTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
					ImGui::TableSetupColumn("Module");
					ImGui::TableSetupColumn("Enabled");
					ImGui::TableHeadersRow();

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("Performance");
					ImGui::TableSetColumnIndex(1); ImGui::Checkbox("##1", Options::performanceEnabled.ptr());

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("Vanilla2Deferred");
					ImGui::TableSetColumnIndex(1); ImGui::Checkbox("##2", Options::vanilla2DeferredEnabled.ptr());

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("MaterialBinLoader");
					ImGui::TableSetColumnIndex(1); ImGui::Checkbox("##3", Options::materialBinLoaderEnabled.ptr());

#if 0
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0); ImGui::Text("Settings");
					ImGui::TableSetColumnIndex(1); ImGui::Checkbox("##4", &Options::windowSettingsEnabled.ptr());
#endif

					//ImGui::TableNextRow();
					//ImGui::TableSetColumnIndex(0); ImGui::Text("CustomUniforms");
					//ImGui::TableSetColumnIndex(1); ImGui::Checkbox("", &Options::customUniformsEnabled.ptr());

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
				ImGui::Text("V1.3.4\n Support 1.20.10\n No longer support version below 1.20.10");
				ImGui::Text("V1.3.5\n Support 1.20.32\n Fixed ImGui flickering on some devices\n Removed Vanilla2Deferred option");
				ImGui::Text("V1.3.6\n Support 1.20.40 and 1.20.50\n Added Force Enable Deferred Technical Preview option");
				ImGui::Text("V1.3.7\n Support 1.20.60.23 preview");
				ImGui::Text("V1.4.0\n Added Reload shaders button");
				ImGui::Text("V1.4.1\n Support 1.20.80.20 preview and 1.20.80.21 preview");
				ImGui::Text("V1.4.2\n Support 1.20.71.01");
				ImGui::Text("V1.4.3\n Support 1.20.80.05\n Removed key binding for reload shaders");
			}
			ImGui::End();
		}

		//if (showDemo)
		//	ImGui::ShowDemoWindow(&showDemo);
	}
	ImGui::EndFrame();

	if (resetLayout) {
		ImGui::ClearWindowSettings("BetterRenderDragon");
		ImGui::ClearWindowSettings("BetterRenderDragon - Module Manager");
		ImGui::ClearWindowSettings("Dear ImGui Demo");
	}
}

bool isChangingUIKey = false;
bool justChangedKey = false;

void updateOptions() {
	static float saveTimer = 0.0f;

	if (Options::isDirty()) {
		saveTimer = 3.0f;
	}
	Options::record();

	//TODO: Put it on a separate thread
	if (saveTimer > 0.0f) {
		saveTimer -= ImGui::GetIO().DeltaTime;
		if (saveTimer <= 0.0f) {
			saveTimer = 0.0f;
			Options::save();
		}
	}
}

void updateKeys() {
	static bool prevToggleImGui = false;

	bool toggleImGui = ImGui::IsKeyPressed((ImGuiKey)Options::uiKey.get()) && !justChangedKey;
	if (!toggleImGui)
		justChangedKey = false;
	if (toggleImGui && !prevToggleImGui)
		Options::showImGui = !Options::showImGui;
	prevToggleImGui = toggleImGui;

	if (Options::reloadShadersAvailable && !Options::reloadShaders && ImGui::IsKeyPressed((ImGuiKey)Options::reloadShadersKey.get())) {
		Options::reloadShaders = true;
	}
}
