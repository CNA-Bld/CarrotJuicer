// dllmain.cpp : Defines the entry point for the DLL application.
#include <cstdio>
#include <locale>
#include <filesystem>
#include <thread>

#include "Windows.h"

extern void attach();
extern void detach();

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD ul_reason_for_call,
                      LPVOID lpReserved
)
{
	WCHAR buffer[MAX_PATH];
	const std::filesystem::path module_path(std::wstring(buffer, GetModuleFileName(nullptr, buffer, MAX_PATH)));

	if (module_path.filename() == L"umamusume.exe")
	{
		current_path(module_path.parent_path());

		if (ul_reason_for_call == DLL_PROCESS_ATTACH)
			std::thread(attach).detach();

		if (ul_reason_for_call == DLL_PROCESS_DETACH)
			detach();
	}
	return TRUE;
}
