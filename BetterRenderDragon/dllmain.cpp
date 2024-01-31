#include <windows.h>
#include <wrl.h>
#pragma comment(lib, "runtimeobject.lib")

#include "ImGuiHooks.h"
#include "MCHooks.h"
#include "MCPatches.h"
#include "Options.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
		case DLL_PROCESS_ATTACH: {
			if (FAILED(RoInitialize(RO_INIT_MULTITHREADED))) {
				printf("RoInitialize failed\n");
				return TRUE;
			}
			Options::init();
			Options::load();

			initMCHooks();
			initMCPatches();
			initImGuiHooks();
			break;
		}
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			Options::save();
			RoUninitialize();
			break;
    }
    return TRUE;
}
