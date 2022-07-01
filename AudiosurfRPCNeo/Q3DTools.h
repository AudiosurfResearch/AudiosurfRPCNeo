#pragma once
#pragma comment( lib, "HighPoly" )

class EngineInterface;
class A3d_Channel;
class A3d_ChannelGroup;
class Aco_FloatChannel;

//HighPoly.dll (core engine of Quest3D) exposes a globalEngine variable we can use
extern	__declspec(dllimport) EngineInterface* globalEngine;

class Q3DTools
{
public:
	Q3DTools();

	static A3d_Channel* GetChannelFromGroup(int groupIndex, int channelIndex);
	static float GetFloatFromChannel(Aco_FloatChannel* floatChannel);

	inline static Aco_FloatChannel* isPausedChannel = nullptr;
	inline static bool isPaused = false;
};