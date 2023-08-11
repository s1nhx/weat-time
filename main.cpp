#include "main.h"

mINI::INIFile file("weatntime.ini");
mINI::INIStructure ini;

void set_weather(int id) {
	*reinterpret_cast<int*>(0xC81320) = id;
}

void set_time(int hour) {
	BitStream bs;
	bs.Write<UINT8>(hour);
	SAMP::pSAMP->getRakNet()->EmulRPC(RPC_ScrSetPlayerTime, &bs);
}

void _cdecl cmd(char* params) {
	int weatID = atoi(params);
	ini["sinhxxx"]["weather"] = std::to_string(weatID);
	file.write(ini);
	set_weather(weatID);
	SAMP::pSAMP->addMessageToChat(-1, "нова€ погода: %d", weatID);
}

void _cdecl cmd2(char* params) {
	int hour = atoi(params);
	if (hour < 0 || hour > 23) {
		SAMP::pSAMP->addMessageToChat(-1, "введено неверное врем€ (0-23)");
		return;
	}
	ini["sinhxxx"]["time"] = std::to_string(hour);
	file.write(ini);
	set_time(hour);
	SAMP::pSAMP->addMessageToChat(-1, "новое врем€: %d", hour);
}

bool __stdcall rpcrecvhook(SAMP::CallBacks::HookedStructs::stRakClientRPCRecv* params) {
	if (params->rpc_id == RPC_ScrSetPlayerTime ||
		params->rpc_id == RPC_ScrSetWorldTime ||
		params->rpc_id == RPC_ScrSetWeather) {
		return false;
	}

	if (params->rpc_id == RPC_ScrClientMessage) {
		set_weather(stoi(ini["sinhxxx"]["weather"]));
		set_time(stoi(ini["sinhxxx"]["time"]));
	}

	return true;
}

void __stdcall GameLoop() {
	static bool initialized = false;
	if (!initialized) {
		if (SAMP::pSAMP->LoadAPI()) {
			initialized = true;
			file.read(ini);

			SAMP::pSAMP->addMessageToChat(-1, "[weat n' time 1.0] loaded. cmd: /.w [id] & /.t [hour] | vk/@sinhxxx");
			SAMP::pSAMP->addClientCommand(".w", cmd);
			SAMP::pSAMP->addClientCommand(".t", cmd2);
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
		SAMP::CallBacks::pCallBackRegister->RegisterGameLoopCallback(GameLoop);//register gameloop hook
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