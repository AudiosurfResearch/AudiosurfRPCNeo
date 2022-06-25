#include "pch.h"
#include "RPCManager.h"

RPCManager::RPCManager()
{
	//init Discord RPC
	auto result = discord::Core::Create(698844368953671750, DiscordCreateFlags_Default, &core);
	core->ActivityManager().RegisterSteam(12900);
	core->UserManager().OnCurrentUserUpdate.Connect([this]() {
		core->UserManager().GetCurrentUser(&currentUser);
		std::cout << currentUser.GetUsername() << "#" << currentUser.GetDiscriminator() << " has logged in!" << "\n";
	});
	UpdatePresence("Loading", "");
}

void RPCManager::UpdatePresence(std::string state, std::string details)
{
	discord::Activity activity{};
	activity.SetState(state.c_str());
	activity.SetDetails(details.c_str());
	activity.GetAssets().SetLargeImage("audiosurficon4x");
	activity.GetAssets().SetLargeText("Audiosurf");
	core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {

	});
}

void RPCManager::RunCallbacks() {
	core->RunCallbacks();
}