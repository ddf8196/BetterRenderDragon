#pragma once

void initMCPatches();

class ScopedVirtualProtect {
public:
    ScopedVirtualProtect(void* addr, size_t size, DWORD newProtect, bool instruction = true) : addr(addr), size(size), instruction(instruction) {
        restore = VirtualProtect(addr, size, newProtect, &oldProtect);
    }
    ~ScopedVirtualProtect() {
        if (restore) {
            VirtualProtect(addr, size, oldProtect, &oldProtect);
        }
        if (instruction) {
            FlushInstructionCache(GetCurrentProcess(), addr, size);
        }
    }
private:
    void* addr;
    size_t size;
    DWORD oldProtect;
    bool instruction;
    bool restore;
};

#define GLUE1(a, b) a##b
#define GLUE(a, b) GLUE1(a,b)
#define ScopedVP(ptr, ...) ScopedVirtualProtect GLUE(svp,__LINE__) ((void*)(ptr), __VA_ARGS__);
