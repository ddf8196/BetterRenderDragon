#include "Util.h"
#include <windows.h>
#include <intrin.h>

std::string wstringToString(const std::wstring& wstr) {
	int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
	char* buffer = new char[len] {};
	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), (int)wstr.size(), buffer, len, nullptr, nullptr);
	std::string result(buffer, len);
	delete[] buffer;
	return result;
}

std::string getCPUName() {
	int CPUInfo[4] = { -1 };
	char CPUBrandString[0x41];
	__cpuid(CPUInfo, 0x80000000);
	unsigned int nExIds = CPUInfo[0];

	memset(CPUBrandString, 0, sizeof(CPUBrandString));

	// Get the information associated with each extended ID.
	for (unsigned int i = 0x80000000; i <= nExIds; ++i) {
		__cpuid(CPUInfo, i);
		// Interpret CPU brand string.
		if (i == 0x80000002)
			memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
		else if (i == 0x80000003)
			memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
		else if (i == 0x80000004)
			memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
	}

	CPUBrandString[0x40] = 0;
	return std::string(CPUBrandString);
}