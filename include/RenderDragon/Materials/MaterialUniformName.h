#pragma once
#include <cstdint>
#include <string>
#include <functional>

namespace dragon::materials {
    typedef uint16_t ParameterId;

    struct MaterialUniformName {
        uint64_t mHash;

        MaterialUniformName(const std::string& name) {
            this->mHash = getHash(name);
        }

        uint64_t getHash(const std::string& name) {
            return std::hash<std::string>{}(name);
        }

        bool operator==(const MaterialUniformName& other) {
            return this->mHash == other.mHash;
        }
    };
}
