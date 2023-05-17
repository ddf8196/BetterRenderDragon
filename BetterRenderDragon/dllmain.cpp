#include <Windows.h>

#include "ImGuiHooks.h"
#include "MCHooks.h"
#include "MCPatches.h"
#include "Options.h"

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
    switch (ul_reason_for_call) {
		case DLL_PROCESS_ATTACH:
			Options::load();

			MCHooks_Init();
			MCPatches_Init();
			ImGuiHooks_Init();
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			Options::save();
			break;
    }
    return TRUE;
}
