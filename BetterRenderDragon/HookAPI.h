#pragma once

#include <cstdio>
#include <string>
#include <utility>
#include <vector>
#include <initializer_list>

#include <windows.h>
#include <psapi.h>
#include <detours/detours.h>

#define FindSignature(signature) FindSig("Minecraft.Windows.exe", signature)

#define DeclareHook(name, ret, ...) \
struct _Hook_##name { \
    static ret (*_original)(__VA_ARGS__); \
    template <typename ...Params> \
    static ret original(Params&&... params) { return (*_original)(std::forward<Params>(params)...); } \
    static ret _hook(__VA_ARGS__); \
}; \
ret (*_Hook_##name::_original)(__VA_ARGS__) = nullptr; \
ret _Hook_##name::_hook(__VA_ARGS__)

#define CallOriginal(name, ...) _Hook_##name::original(__VA_ARGS__)
#define IsHooked(name) (_Hook_##name::_original != nullptr)
#define Hook(name, ptr) HookFunction(ptr, (void**)&_Hook_##name::_original, &_Hook_##name::_hook)
#define Unhook(name) UnhookFunction(_Hook_##name::_original, &_Hook_##name::_hook)
#define TrySigHook(name, ...) SigHook(#name, __VA_ARGS__, (void**)&_Hook_##name::_original, &_Hook_##name::_hook)
#define TrySigHookNoWarning(name, ...) SigHook(#name, __VA_ARGS__, (void**)&_Hook_##name::_original, &_Hook_##name::_hook, false)

inline int HookFunction(void* oldFunc, void** outOldFunc, void* newFunc) {
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    int rv = DetourAttach(&oldFunc, newFunc);
    DetourTransactionCommit();
    *outOldFunc = oldFunc;
    return rv;
}

inline int UnhookFunction(void* oldFunc, void* newFunc) {
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    int rv = DetourDetach(&oldFunc, newFunc);
    DetourTransactionCommit();
    return rv;
}

inline void ReplaceVtable(void* _vptr, size_t index, void** outOldFunc, void* newFunc) {
    void** vptr = (void**)_vptr;
    void* oldFunc = vptr[index];
    if (oldFunc == newFunc) {
        return;
    }
    if (outOldFunc != nullptr) {
	    *outOldFunc = oldFunc;
    }

    DWORD oldProtect, tmp;
    VirtualProtect(vptr + index, sizeof(void*), PAGE_READWRITE, &oldProtect);
    vptr[index] = newFunc;
    VirtualProtect(vptr + index, sizeof(void*), oldProtect, &tmp);
}

inline uintptr_t FindSig(const std::string& moduleName, const std::string& signature) {
    HMODULE moduleHandle = GetModuleHandleA(moduleName.c_str());
    MODULEINFO moduleInfo;
    GetModuleInformation(GetCurrentProcess(), moduleHandle, &moduleInfo, sizeof(MODULEINFO));

    std::vector<uint16_t> pattern;
    for (int i = 0; i < signature.size(); i++) {
        if (signature[i] == ' ')
            continue;
        if (signature[i] == '?') {
            pattern.push_back(0xFF00);
            i++;
        } else {
            char buf[3] { signature[i], signature[++i], 0 };
            pattern.push_back((uint16_t)strtoul(buf, nullptr, 16));
        }
    }

    if (pattern.size() == 0)
        return (uintptr_t)moduleHandle;

    int patternIdx = 0;
    uintptr_t match = 0;
    for (uintptr_t i = (uintptr_t)moduleHandle; i < (uintptr_t)moduleHandle + moduleInfo.SizeOfImage; i++) {
        uint8_t current = *(uint8_t*)i;
        if (current == pattern[patternIdx] || pattern[patternIdx] & 0xFF00) {
            if (!match)
                match = i;
            patternIdx++;
            if (patternIdx == pattern.size()) {
                return match;
            }
        } else {
            if (match) {
                i--;
            }
            match = 0;
            patternIdx = 0;
        }
    }

    return 0;
}

inline bool SigHook(const std::string& name, const std::initializer_list<std::string>& signatures, void** outOldFunc, void* newFunc, bool warnIfFailed = true) {
    uintptr_t ptr = 0;
    for (auto& sig : signatures) {
        if (ptr = FindSignature(sig)) {
            break;
        }
    }
    if (!ptr) {
        if (warnIfFailed) {
            printf("Failed to hook %s (signature not found)\n", name.c_str());
        }
        return false;
    }
    if (HookFunction((void*)ptr, outOldFunc, newFunc) != NO_ERROR) {
        if (warnIfFailed) {
            printf("Failed to hook %s\n", name.c_str());
        }
        return false;
    }
    return true;
}