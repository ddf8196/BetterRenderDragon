#include "Util.h"
#include <locale.h>
#include <intrin.h>

std::string wstringToString(const std::wstring& str) {
	unsigned len = str.size() * 4;
	setlocale(LC_CTYPE, "");
	char* p = new char[len];
	size_t numCharConv;
	wcstombs_s(&numCharConv, p, len, str.c_str(), len);
	std::string str1(p);
	delete[] p;
	return str1;
}

std::string getCPUName() {
	int CPUInfo[4] = { -1 };
	char CPUBrandString[0x41];
	__cpuid(CPUInfo, 0x80000000);
	unsigned int nExIds = CPUInfo[0];

	memset(CPUBrandString, 0, sizeof(CPUBrandString));

	// Get the information associated with each extended ID.
	for (int i = 0x80000000; i <= nExIds; ++i) {
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