#pragma once
#include <cstdint>
#include <string>

namespace dragon {
	namespace materials {
        struct MaterialUniformName {
	        uint64_t mHash;

            MaterialUniformName(const std::string& name) {
                //FNV-1a 64
                uint64_t hash = 0xCBF29CE484222325;
                for (uint8_t c : name) {
                    hash ^= c;
                    hash *= 0x100000001B3;
                }
                this->mHash = hash;
            }

            bool operator==(const MaterialUniformName& other) {
                return this->mHash == other.mHash;
            }
        };
    }
}
