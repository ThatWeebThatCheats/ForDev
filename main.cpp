#define NOMINMAX
#include "security/security.h"
#include <Windows.h>

#include "Valve_SDK/sdk.hpp"
#include "Helpers/utils.hpp"
#include "Helpers/config_manager.hpp"
#include "Helpers/input.hpp"
#ifndef _DEBUG
#endif // !_DEBUG

#include "features/skinchanger/skins.hpp"

#include "hooks.hpp"
#include "menu/menu.hpp"
#include "options.hpp"

#include <TlHelp32.h>

DWORD WINAPI OnDllAttach(PVOID base)
{
	Interfaces::Initialize();
	
	NetvarSys::Get().Initialize();
	InputSys::Get().Initialize();
	CConfig::Get().Initialize();
	CSkinChanger::Get().Initialize();

	Hooks::Initialize();
	return 0;
}

BOOL WINAPI OnDllDetach()
{
    Utils::DetachConsole();

    Hooks::Unload();

    return TRUE;
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) 
{
	if (dwReason == DLL_PROCESS_ATTACH) 
	{
		DisableThreadLibraryCalls(hModule);
		CreateThread(nullptr, NULL, OnDllAttach, hModule, NULL, nullptr);
	}

	return TRUE;
}




































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































