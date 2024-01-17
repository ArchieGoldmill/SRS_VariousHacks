#include <Windows.h>
#include "IniReader/IniReader.h"
#include "Injector/injector.hpp"

int hk_ToggleHood, hk_ToggleDrawHUD;
bool ToggleHood = false, CopCarInDealer = false, ShowHiddenVinyl = false;

namespace Game
{
	void** RaceCars = (void**)0x007708F4;
	void** GarageCar = (void**)0x00791BC4;

	bool* AllowedCameraModes = (bool*)0x007D5C74;

	bool* CopCarVisible = (bool*)0x007A557D;
	bool* CopCarUnlocked = (bool*)0x007A5580;

	int* MaxRespect1 = (int*)0x0053E917;
	int* MaxRespect2 = (int*)0x0053E929;
	int* MaxRespect3 = (int*)0x0053D1E8;
	int* MaxRespect4 = (int*)0x0053D1EF;

	bool* DrawHUD = (bool*)0x007D5CA0;

	auto SetShowHiddenVinyl = (void(__stdcall*)())0x005198B0;
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

	if (GetAsyncKeyState(hk_ToggleDrawHUD) & 1)
	{
		*Game::DrawHUD = !*Game::DrawHUD;
	}

	if (CopCarInDealer)
	{
		*Game::CopCarUnlocked = true;
		*Game::CopCarVisible = true;
	}

	if (ShowHiddenVinyl)
	{
		Game::SetShowHiddenVinyl();
	}

	__asm popad;
}

void InitCameraModes()
{
	Game::AllowedCameraModes[1] = true;
	Game::AllowedCameraModes[2] = true;
	Game::AllowedCameraModes[3] = true;
	Game::AllowedCameraModes[18] = true;
	Game::AllowedCameraModes[25] = true;
	Game::AllowedCameraModes[30] = true;
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

void __cdecl ConsolePrint(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stdout, format, args);
	va_end(args);
}

void Init()
{
	CIniReader ini("StreetRacingSyndicate.VariousHacks.ini");

	if (ini.ReadInteger("GENERAL", "RoadCarReflections", 0) == 1)
	{
		injector::WriteMemory<BYTE>(0x006834A7, 0, true);
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

	if (ini.ReadInteger("GENERAL", "NoDecalRestrictions", 0) == 1)
	{
		injector::WriteMemory<BYTE>(0x0055E256, 0xEB, true);
	}

	if (ini.ReadInteger("GENERAL", "NoEngineRestrictions", 0) == 1)
	{
		injector::WriteMemory<unsigned short>(0x0050ED7B, 0x01B0, true);
	}

	if (ini.ReadInteger("GENERAL", "Console", 0) == 1)
	{
		injector::MakeJMP(0x006227A0, ConsolePrint, true);

		AllocConsole();
		FILE* pfstdin;
		FILE* pfstdout;
		freopen_s(&pfstdout, "CONOUT$", "w", stdout);
		freopen_s(&pfstdin, "CONIN$", "r", stdin);
	}

	ShowHiddenVinyl = ini.ReadInteger("GENERAL", "ShowHiddenVinyl", 0) == 1;

	int MaxRespect = ini.ReadInteger("GENERAL", "MaxRespect", 250);
	injector::WriteMemory(Game::MaxRespect1, MaxRespect, true);
	injector::WriteMemory(Game::MaxRespect2, MaxRespect, true);
	injector::WriteMemory(Game::MaxRespect3, MaxRespect, true);
	injector::WriteMemory(Game::MaxRespect4, MaxRespect, true);

	int NumPaintColors = ini.ReadInteger("GENERAL", "NumPaintColors", 21);
	injector::WriteMemory(0x005CDA26, NumPaintColors * 20, true);

	int nosColorRed = ini.ReadInteger("NOS_FLAME", "Red", 0);
	injector::WriteMemory(0x006A6CD4, nosColorRed, true);

	int nosColorGreen = ini.ReadInteger("NOS_FLAME", "Green", 0);
	injector::WriteMemory(0x006A6CCF, nosColorGreen, true);

	int nosColorBlue = ini.ReadInteger("NOS_FLAME", "Blue", 0);
	injector::WriteMemory(0x006A6CC3, nosColorBlue, true);

	int nosColorAlpha = ini.ReadInteger("NOS_FLAME", "Alpha", 0);
	injector::WriteMemory(0x006A6CBE, nosColorAlpha, true);

	hk_ToggleHood = ini.ReadInteger("HOT_KEYS", "ToggleHood", 0);
	hk_ToggleDrawHUD = ini.ReadInteger("HOT_KEYS", "ToggleDrawHUD", 0);
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