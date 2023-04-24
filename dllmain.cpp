#include <Windows.h>
#include "IniReader/IniReader.h"
#include "Injector/injector.hpp"

int hk_ToggleHood;
bool ToggleHood = false, CopCarInDealer = false;

namespace Game
{
	void** RaceCars = (void**)0x007708F4;
	void** GarageCar = (void**)0x00791BC4;

	bool* AllowedCameraModes = (bool*)0x007D5C74;

	bool* CopCarVisible = (bool*)0x007A557D;
	bool* CopCarUnlocked = (bool*)0x007A5580;

	auto ProcessEngineAnimation = (void(__thiscall*)(void*, int))0x00541DF0;
	auto SetEngineAnimationState = (void(__thiscall*)(void*, int))0x00541D20;
	auto IsEngineAnimationState = (bool(__thiscall*)(void*, int))0x0053F5D0;
}

void ProcessEngineAnimation(void* playerCar)
{
	if (playerCar)
	{
		Game::ProcessEngineAnimation(playerCar, 0);
		if (!ToggleHood)
		{
			if (Game::IsEngineAnimationState(playerCar, 0))
			{
				Game::SetEngineAnimationState(playerCar, 2);
			}
		}
		else if (!Game::IsEngineAnimationState(playerCar, 0))
		{
			Game::SetEngineAnimationState(playerCar, 2);
		}
	}
}

void MainLoop()
{
	__asm pushad;

	ProcessEngineAnimation(Game::RaceCars[0]);
	ProcessEngineAnimation(Game::GarageCar[0]);

	if (GetAsyncKeyState(hk_ToggleHood) & 1)
	{
		ToggleHood = !ToggleHood;
	}

	if (CopCarInDealer)
	{
		*Game::CopCarUnlocked = true;
		*Game::CopCarVisible = true;
	}

	__asm popad;
}

void InitCameraModes()
{
	Game::AllowedCameraModes[1] = true;
	Game::AllowedCameraModes[2] = true;
	Game::AllowedCameraModes[3] = true;
	Game::AllowedCameraModes[18] = true;
}

void __declspec(naked) InitCameraModesCave()
{
	__asm
	{
		pushad;
		call InitCameraModes;
		popad;
		retn;
	}
}

void Init()
{
	CIniReader ini("StreetRacingSyndicate.VariousHacks.ini");

	if (ini.ReadInteger("GENERAL", "CarReflections", 0) == 1)
	{

	}

	CopCarInDealer = ini.ReadInteger("GENERAL", "CopCarInDealer", 0) == 1;

	if (ini.ReadInteger("GENERAL", "HighestLods", 0) == 1)
	{
		injector::MakeNOP(0x00551B12, 2, true);
	}

	if (ini.ReadInteger("GENERAL", "InfiniteNosFlame", 0) == 1)
	{
		injector::MakeNOP(0x006A86C4, 2, true);
	}

	if (ini.ReadInteger("GENERAL", "MoreCameraModes", 0) == 1)
	{
		injector::MakeJMP(0x00523659, InitCameraModesCave, true);
	}

	int nosColorRed = ini.ReadInteger("NOS_FLAME", "Red", 0);
	injector::WriteMemory(0x006A6CD4, nosColorRed, true);

	int nosColorGreen = ini.ReadInteger("NOS_FLAME", "Green", 0);
	injector::WriteMemory(0x006A6CCF, nosColorGreen, true);

	int nosColorBlue = ini.ReadInteger("NOS_FLAME", "Blue", 0);
	injector::WriteMemory(0x006A6CC3, nosColorBlue, true);

	int nosColorAlpha = ini.ReadInteger("NOS_FLAME", "Alpha", 0);
	injector::WriteMemory(0x006A6CBE, nosColorAlpha, true);

	hk_ToggleHood = ini.ReadInteger("HOT_KEYS", "ToggleHood", 0);
	injector::MakeCALL(0x004044B8, MainLoop, true);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  reason, LPVOID lpReserved)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		Init();
	}

	return TRUE;
}