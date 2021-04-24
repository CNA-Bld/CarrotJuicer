// dllmain.cpp : Defines the entry point for the DLL application.
#include <cstdio>
#include <locale>
#include <filesystem>
#include "Windows.h"

extern void attach();
extern void detach();

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD ul_reason_for_call,
                      LPVOID lpReserved
)
{
	char buffer[MAX_PATH];
	std::filesystem::path module_path(std::string(buffer, GetModuleFileName(nullptr, buffer, MAX_PATH)));

	if (module_path.filename() == "umamusume.exe")
	{
		current_path(module_path.parent_path());

		if (ul_reason_for_call == DLL_PROCESS_ATTACH)
			attach();

		if (ul_reason_for_call == DLL_PROCESS_DETACH)
			detach();
	}
	return TRUE;
}
