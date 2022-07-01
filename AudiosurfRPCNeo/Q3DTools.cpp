#include "pch.h"
#include "Q3DTools.h"

static A3d_ChannelGroup* (__thiscall* EngineInterface_GetChannelGroup)(EngineInterface* self, int);
static A3d_Channel* (__thiscall* ChannelGroup_GetChannel)(A3d_ChannelGroup* self, int);
static float (__thiscall* Aco_FloatChannel_GetFloat)(Aco_FloatChannel* self);

Q3DTools::Q3DTools()
{
	EngineInterface_GetChannelGroup = (A3d_ChannelGroup * (__thiscall*)(EngineInterface*, int))
		GetProcAddress(GetModuleHandle("HighPoly.dll"), "?GetChannelGroup@EngineInterface@@UAEPAVA3d_ChannelGroup@@H@Z");

	ChannelGroup_GetChannel = (A3d_Channel * (__thiscall*)(A3d_ChannelGroup*, int))
		GetProcAddress(GetModuleHandle("HighPoly.dll"), "?GetChannel@A3d_ChannelGroup@@UAEPAVA3d_Channel@@H@Z");

	Aco_FloatChannel_GetFloat = (float(__thiscall*)(Aco_FloatChannel*))
		GetProcAddress(GetModuleHandle("BE69CCC4-CFC1-4362-AC81-767D199BBFC3.dll"), "?GetFloat@Aco_FloatChannel@@UAEMXZ");
}

A3d_Channel* Q3DTools::GetChannelFromGroup(int groupIndex, int channelIndex) {
	return ChannelGroup_GetChannel(EngineInterface_GetChannelGroup(globalEngine, groupIndex), channelIndex);
}

float Q3DTools::GetFloatFromChannel(Aco_FloatChannel* floatChannel) {
	return Aco_FloatChannel_GetFloat(floatChannel);
}