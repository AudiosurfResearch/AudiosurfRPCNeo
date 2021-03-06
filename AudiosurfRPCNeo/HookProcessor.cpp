#include "pch.h"
#include "HookProcessor.h"
#include "RPCManager.h"
#include "Q3DTools.h"

LRESULT (WINAPI* TrueSendMessage)(
	HWND hWnd,
	UINT   Msg,
	WPARAM wParam,
	LPARAM lParam) = SendMessageA;
TagLib::Tag* (__thiscall* TrueFileRefGetTag)(TagLib::FileRef* self) = nullptr;
void (__thiscall* TrueSetFloat)(Aco_FloatChannel* self, float newFloat) = nullptr;

bool windowRegistered;

TagLib::Tag* tagPtr;
std::string artistTag = std::string("Unknown Artist");
std::string titleTag = std::string("Unknown Title");
std::string fullTags = std::string("Unknown Artist - Unknown Title");

std::string score;
std::string fullScore;
std::string songFinishText;

LRESULT WINAPI MessageInterceptor(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	COPYDATASTRUCT* cds = (COPYDATASTRUCT*)lParam;
	std::cout << "Outgoing message caught: " << (char*)cds->lpData << "\n";

	if (strstr((char*)cds->lpData, (char*)"registered")) {
		std::cout << "WM_COPYDATA successfully registered\n";
		windowRegistered = true;
	}

	if (strstr((char*)cds->lpData, (char*)"oncharacterscreen")) {
		std::cout << "Entered character select screen" << "\n";
		RPCManager::globalRPCManager->UpdatePresence("Selecting character/song", "");
	}

	if (strstr((char*)cds->lpData, (char*)"nowplayingartistname")) {
		std::cout << "Playing song: " << artistTag << " - " << titleTag << "\n";

		RPCManager::globalRPCManager->UpdatePresence(fullTags, "Riding song");
	}

	if (strstr((char*)cds->lpData, (char*)"songcomplete")) {
		score = (char*)cds->lpData;
		//Make only the score number remain in the stuff the game sent back
		score.erase(0, 22);
		fullScore = score + " points";
		songFinishText = "Song finished: " + fullTags;
		std::cout << "Song complete: " << artistTag << " - " << titleTag << " with score " << score << "\n";

		RPCManager::globalRPCManager->UpdatePresence(fullScore, songFinishText);
	}

	return TrueSendMessage(hWnd, Msg, wParam, lParam);
}

static TagLib::Tag* __fastcall FileRefGetTagInterceptor(TagLib::FileRef* self, DWORD edx) {
	std::cout << Q3DTools::GetFloatFromChannel(reinterpret_cast<Aco_FloatChannel*>(Q3DTools::GetChannelFromGroup(54, 28))) << std::endl;

	tagPtr = TrueFileRefGetTag(self);
	artistTag = tagPtr->artist().toCString(true);
	titleTag = tagPtr->title().toCString(true);
	if (artistTag.empty()) {
		artistTag = "Unknown Artist";
	}
	if (titleTag.empty()) {
		titleTag = "Unknown Title";
	}
	fullTags = artistTag + std::string(" - ") + titleTag;

	std::cout << "TagLib::FileRef::tag() caught! " << fullTags.c_str() << "\n";

	if (!windowRegistered) {
		std::cout << "FileRef received but the window isn't registered! Attempting registration...";
		char* str = (char*)"ascommand registerlistenerwindow Audiosurf";
		COPYDATASTRUCT cds;
		cds.cbData = strlen(str) + 1;
		cds.lpData = (void*)str;
	}

	//Get the channel that stores if the game is paused
	if (!Q3DTools::isPausedChannel) Q3DTools::isPausedChannel = (Aco_FloatChannel*)Q3DTools::GetChannelFromGroup(54, 28);

	return tagPtr;
}

static void __fastcall SetFloatInterceptor(Aco_FloatChannel* self, DWORD edx, float newFloat) {
	if (self == Q3DTools::isPausedChannel) {
		std::cout << "Game pause state changed to " << newFloat << "\n";
		if (newFloat != 0) {
			Q3DTools::isPaused = true;
			RPCManager::globalRPCManager->UpdatePresence(fullTags, "Paused");
		}
		else {
			Q3DTools::isPaused = false;
			RPCManager::globalRPCManager->UpdatePresence(fullTags, "Riding song");
		}
	}

	TrueSetFloat(self, newFloat);
}

HookProcessor::HookProcessor() {
	//get Audiosurf's window handle
	HWND hwndTargetWin = FindWindow(NULL, "Audiosurf");
	while (!hwndTargetWin)
	{
		std::cout << "Couldn't get Audiosurf window handle, retrying in 500ms.\n";
		Sleep(500);
		hwndTargetWin = FindWindow(NULL, "Audiosurf");
	}

	DetourRestoreAfterWith();

	TrueFileRefGetTag =
		(TagLib::Tag * (__thiscall*)(TagLib::FileRef*))
		DetourFindFunction("taglib.dll", "?tag@FileRef@TagLib@@QBEPAVTag@2@XZ");

	TrueSetFloat =
		(void(__thiscall*)(Aco_FloatChannel*, float))
		DetourFindFunction("BE69CCC4-CFC1-4362-AC81-767D199BBFC3.dll", "?SetFloat@Aco_FloatChannel@@UAEXM@Z");

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)TrueSendMessage, MessageInterceptor);
	DetourAttach(&(PVOID&)TrueFileRefGetTag, FileRefGetTagInterceptor);
	DetourAttach(&(PVOID&)TrueSetFloat, SetFloatInterceptor);
	DetourTransactionCommit();

	//create the command message and data struct
	char* str = (char*)"ascommand quickstartregisterwindow Audiosurf";
	COPYDATASTRUCT cds;
	cds.cbData = strlen(str) + 1;
	cds.lpData = (void*)str;

	SendMessage(hwndTargetWin, WM_COPYDATA, 0, (LPARAM)&cds);

	//Initialize Quest3D interface
	Q3DTools::Q3DTools();
}

HookProcessor::~HookProcessor() {
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	//Detach the hooks when the DLL is ejected
	DetourDetach(&(PVOID&)TrueSendMessage, MessageInterceptor);
	DetourDetach(&(PVOID&)TrueFileRefGetTag, FileRefGetTagInterceptor);
	DetourDetach(&(PVOID&)TrueSetFloat, SetFloatInterceptor);
	DetourTransactionCommit();
}