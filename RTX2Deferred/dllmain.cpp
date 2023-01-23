#include <Windows.h>

#include "D3D12Hooks.h"
#include "MCHooks.h"
#include "MCPatches.h"

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
    switch (ul_reason_for_call) {
		case DLL_PROCESS_ATTACH:
			D3D12Hooks_Init();
			MCHooks_Init();
			MCPatches_Init();
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}
