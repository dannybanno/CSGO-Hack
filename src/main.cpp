#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <thread>

// expose our cheat to main.cpp
#include "core/hooks.h"
#include "../ext/DiscordRPC/discord_rpc.h"
#include "../ext/imgui/imgui.h"
//#include "menu/gui.h"

void start() {
	DiscordEventHandlers Handler;
	memset(&Handler, 0, sizeof(Handler));
	Discord_Initialize("CLIENT ID", &Handler, 1, NULL);
}

void update() {
	DiscordRichPresence discordPresence;
	memset(&discordPresence, 0, sizeof(discordPresence));
	while(true){
		discordPresence.details = "Danny's CSGO Cheat";
		discordPresence.startTimestamp = 1507665886;
		discordPresence.largeImageKey = "me_2_";
		discordPresence.smallImageKey = "me_2_";
		Discord_UpdatePresence(&discordPresence);
	}
}

// setup our cheat & unload it when exit key is pressed
DWORD WINAPI Setup(LPVOID lpParam)
{
	//AllocConsole();                                 //
	//FILE* file;									  //	Opens Console
	//freopen_s(&file, "CONOUT$", "w", stdout);	      //

	memory::Setup();		// find signatures
	interfaces::Setup();    // capture interfaces
	netvars::Setup();		// dump latest offsets
	gui::Setup();			// setup gui
	hooks::SetupGUI();		// hook gui
	hooks::Setup();			// place hooks

	start();				//start discord rpc
	update();				//update rpc

	// sleep our thread until unload key is pressed
	while (!GetAsyncKeyState(VK_END))
		std::this_thread::sleep_for(std::chrono::milliseconds(200));

	hooks::Destroy();		// restore hooks
	gui::Destroy();

	// unload library
	FreeLibraryAndExitThread(static_cast<HMODULE>(lpParam), EXIT_SUCCESS);
}

BOOL APIENTRY DllMain(HMODULE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	// dll is being loaded
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		// disable thread notifications
		DisableThreadLibraryCalls(hinstDLL);

		// create our "Setup" thread
		const HANDLE hThread = CreateThread(
			nullptr,
			NULL,
			Setup,
			hinstDLL,
			NULL,
			nullptr
		);

		// free thread handle because we have no use for it
		if (hThread)
			CloseHandle(hThread);
	}

	// successful DLL_PROCESS_ATTACH
	return TRUE;
}
