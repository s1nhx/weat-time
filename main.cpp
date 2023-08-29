#include "main.h"

constexpr auto SET_WEATHER_CONST = 0xC81320;

mINI::INIFile file("weatntime.ini"); // name of config file
mINI::INIStructure ini; // initialize config

void set_weather(int id) {
	*reinterpret_cast<int*>(SET_WEATHER_CONST) = id;
}

void set_time(int hour) {
	BitStream bs;
	bs.Write<UINT8>(hour);
	SAMP::pSAMP->getRakNet()->EmulRPC(RPC_ScrSetPlayerTime, &bs);
}

void _cdecl set_weather_cmd(char* params) {
	int weatID = atoi(params);
	ini["sinhxxx"]["weather"] = std::to_string(weatID); // save weather to config
	file.write(ini);
	set_weather(weatID);
	SAMP::pSAMP->addMessageToChat(-1, "новая погода: %d", weatID);
}

void _cdecl set_time_cmd(char* params) {
	int hour = atoi(params);
	if (hour < 0 || hour > 23) {
		SAMP::pSAMP->addMessageToChat(-1, "введено неверное время (0-23)");
		return;
	}
	ini["sinhxxx"]["time"] = std::to_string(hour); // save time to config
	file.write(ini);
	set_time(hour);
	SAMP::pSAMP->addMessageToChat(-1, "новое время: %d", hour);
}

bool __stdcall rpcrecvhook(SAMP::CallBacks::HookedStructs::stRakClientRPCRecv* params) {
	// this if statement nops every call that changes player's time/weather, hence saving..
	// .. player's settings on them
	if (params->rpc_id == RPC_ScrSetPlayerTime ||
		params->rpc_id == RPC_ScrSetWorldTime ||
		params->rpc_id == RPC_ScrSetWeather) {
		return false;
	}

	// securing that player will see only weather and time that written in config (this shit is unstable but idc)
	if (params->rpc_id == RPC_ScrClientMessage) {
		set_weather(stoi(
			ini["sinhxxx"]["weather"] // turn weather id to integer, pass it to fn
		));
		set_time(stoi(
			ini["sinhxxx"]["time"] // turn hour to integer, pass it to fn
		));
	}

	return true;
}

void __stdcall GameLoop() {
	static bool initialized = false;
	if (!initialized) {
		if (SAMP::pSAMP->LoadAPI()) {
			initialized = true;
			file.read(ini); // get values from config

			SAMP::pSAMP->addMessageToChat(-1, "[weat n' time 1.0] loaded. cmd: /.w [id] & /.t [hour] | vk/@sinhxxx");
			SAMP::pSAMP->addClientCommand(".w", set_weather_cmd);
			SAMP::pSAMP->addClientCommand(".t", set_time_cmd);
		}
	}

	if (initialized) {	//gameLoop

	}

}

int __stdcall DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH: {
		SAMP::Init();
		SAMP::CallBacks::pCallBackRegister->RegisterGameLoopCallback(GameLoop);
		SAMP::CallBacks::pCallBackRegister->RegisterRakClientCallback(rpcrecvhook);
		break;
	}
	case DLL_PROCESS_DETACH: {
		SAMP::ShutDown();
		break;
	}
	}
	return true;
}
