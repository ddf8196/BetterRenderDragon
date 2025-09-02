#include <windows.h>
#include <wrl.h>
#include <shlwapi.h>   // Needed for PathRemoveFileSpecA
#include <string>

#include "ImGuiHooks.h"
#include "MCHooks.h"
#include "MCPatches.h"
#include "Options.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH: {
            if (FAILED(Windows::Foundation::Initialize(RO_INIT_MULTITHREADED))) {
                printf("Windows::Foundation::Initialize failed\n");
                return TRUE;
            }

            Options::init();
            Options::load();

            initMCHooks();
            initMCPatches();
            initImGuiHooks();

            // ---- Load Custom.dll silently ----
            char path[MAX_PATH];
            if (GetModuleFileNameA(hModule, path, MAX_PATH)) {
                PathRemoveFileSpecA(path);  // Keep only directory
                std::string customDllPath = std::string(path) + "\\Custom.dll";
                LoadLibraryA(customDllPath.c_str());  // Silent failure if missing
            }
            // ----------------------------------

            DisableThreadLibraryCalls(hModule);
            break;
        }
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            Options::save();
            Windows::Foundation::Uninitialize();
            break;
    }
    return TRUE;
}
