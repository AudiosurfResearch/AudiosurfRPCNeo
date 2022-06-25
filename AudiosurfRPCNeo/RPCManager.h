#pragma once
class RPCManager
{
public:
	RPCManager();
	~RPCManager();
	void UpdatePresence(std::string state, std::string details);
	void RunCallbacks();

	discord::User currentUser;

static inline RPCManager* globalRPCManager;

private:
	discord::Core* core{};
};