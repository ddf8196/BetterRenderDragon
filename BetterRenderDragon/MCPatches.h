#pragma once

void MCPatches_Init();

class ScopedVirtualProtect {
public:
    ScopedVirtualProtect(void* addr, size_t size, DWORD newProtect) : addr(addr), size(size) {
        VirtualProtect(addr, size, newProtect, &oldProtect);
    }
    ~ScopedVirtualProtect() {
        VirtualProtect(addr, size, oldProtect, &oldProtect);
    }
private:
    void* addr;
    size_t size;
    DWORD oldProtect;
};

#define glue1(a, b) a##b
#define glue(a, b) glue1(a,b)
#define scopedVP(ptr, size, newProtect) ScopedVirtualProtect glue(svp,__LINE__) ((void*)(ptr), (size), (newProtect));
