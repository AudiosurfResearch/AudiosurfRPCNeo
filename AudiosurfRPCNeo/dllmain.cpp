// dllmain.cpp : Defines the DLL's entry point.
#include "pch.h"
#include "HookProcessor.h"
#include "RPCManager.h"

std::vector<std::thread> threadVec;

DWORD WINAPI MainThread(HMODULE hModule)
{
	//Create Console
	AllocConsole();
	FILE* f;
	freopen_s(&f, "CONOUT$", "w", stdout);
	std::cout << "DLL attached!\n";

	std::cout << "Waiting for Discord Game SDK to be loaded...\n";
	while (!GetModuleHandle("discord_game_sdk.dll")) {
		Sleep(250);
	}
	std::cout << "Discord Game SDK loaded at " << GetModuleHandle("discord_game_sdk.dll") << std::endl;

	HookProcessor msgProc;
	RPCManager::globalRPCManager = new RPCManager();

	while (true) {
		//Run Discord callbacks
		RPCManager::globalRPCManager->RunCallbacks();
		//i still don't know why i'm using exactly 16ms for this but does it matter
		Sleep(16);
	}
	
	std::cout << "Shutting down!\n";
	msgProc.~HookProcessor();
	fclose(f);
	FreeConsole();

	FreeLibraryAndExitThread(hModule, 0);
	return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
	if (DetourIsHelperProcess()) {
		return TRUE;
	}

    switch (ul_reason_for_call)
    {
	case DLL_PROCESS_ATTACH: {
		std::thread mainThread(MainThread, hModule);
		//Detach main thread so it doesn't scream about abort() being called on game exit
		mainThread.detach();
		threadVec.push_back(std::move(mainThread));
	}
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}