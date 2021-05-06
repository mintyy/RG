#include "Windows.h"
#include <iostream>
#include "MemMan.h"
#include "csgo.hpp"

MemMan MemClass;

using namespace hazedumper::netvars;
using namespace hazedumper::signatures;

struct variables
{
	DWORD localPlayer;
	DWORD gameModule;
	DWORD engineModule;
	DWORD glowObject;
} val;

struct ClrRender
{
	BYTE red, green, blue;
};
ClrRender clrEnemy;
ClrRender clrTeam;

struct GlowStruct
{
	BYTE base[4];
	float red;
	float green;
	float blue;
	float alpha;
	BYTE buffer[16];
	bool renderWhenOccluded;
	bool renderWhenUnOccluded;
	bool fullBloom;
	BYTE buffer1[5];
	int glowStyle;
};

GlowStruct SetGlowColor(GlowStruct Glow, DWORD entity)
{
	bool defusing = MemClass.readMem<bool>(entity + m_bIsDefusing);
	if (defusing)
	{
		Glow.red = 1.0f;
		Glow.green = 1.0f;
		Glow.blue = 1.0f;
	}
	else
	{
		int health = MemClass.readMem<int>(entity + m_iHealth);
		Glow.red = health * -0.01 + 1;
		Glow.green = health * 0.01;
	}
	Glow.alpha = 1.0f;
	Glow.renderWhenOccluded = true;
	Glow.renderWhenUnOccluded = false;
	//Glow.fullBloom = 1.0f;
	return Glow;
}

void SetTeamGlow(DWORD entity, int glowIndex)
{
	GlowStruct TGlow;
	TGlow = MemClass.readMem<GlowStruct>(val.glowObject + (glowIndex * 0x38));
	
	TGlow.blue = 1.0f;
	TGlow.alpha = 1.0f;
	//TGlow.fullBloom = 1.0f;
	TGlow.renderWhenOccluded = true;
	TGlow.renderWhenUnOccluded = false;
	MemClass.writeMem<GlowStruct>(val.glowObject + (glowIndex * 0x38), TGlow);
}


void SetEnemyGlow(DWORD entity, int glowIndex)
{
	GlowStruct EGlow;
	EGlow = MemClass.readMem<GlowStruct>(val.glowObject + (glowIndex * 0x38));
	EGlow = SetGlowColor(EGlow, entity);
	//EGlow.fullBloom = 1.0f;
	MemClass.writeMem<GlowStruct>(val.glowObject + (glowIndex * 0x38), EGlow);
}


void HandleGlow()
{
	val.glowObject = MemClass.readMem<DWORD>(val.gameModule + dwGlowObjectManager);
	int myTeam = MemClass.readMem<int>(val.localPlayer + m_iTeamNum);

	for (short int i = 0; i < 32; i++)
	{
		DWORD entity = MemClass.readMem<DWORD>(val.gameModule + dwEntityList + i * 0x10);
		if (entity != NULL)
		{
			int glowIndx = MemClass.readMem<int>(entity + m_iGlowIndex);
			int entityTeam = MemClass.readMem<int>(entity + m_iTeamNum);

			if (myTeam == entityTeam)
			{
				MemClass.writeMem<ClrRender>(entity + m_clrRender, clrTeam);
				SetTeamGlow(entity, glowIndx);
			}
			else
			{
				MemClass.writeMem<ClrRender>(entity + m_clrRender, clrEnemy);
				SetEnemyGlow(entity, glowIndx);
			}
		}
	}
}

void HandleRadar() {
	val.glowObject = MemClass.readMem<DWORD>(val.gameModule + dwGlowObjectManager);
	int myTeam = MemClass.readMem<int>(val.localPlayer + m_iTeamNum);

	for (short int i = 0; i < 32; i++)
	{
		DWORD entity = MemClass.readMem<DWORD>(val.gameModule + dwEntityList + i * 0x10);
		if (entity != NULL)
		{
			MemClass.writeMem<int>(entity + m_bSpotted, 1);
		}
	}
}

void SetBrightness()
{
	clrTeam.red = 0;
	clrTeam.blue = 255;
	clrTeam.green = 0;

	clrEnemy.red = 255;
	clrEnemy.blue = 0;
	clrEnemy.green = 0;

	float brightness = 5.0f;

	int ptr = MemClass.readMem<int>(val.engineModule + model_ambient_min);
	int xorptr = *(int*)&brightness ^ ptr;
	MemClass.writeMem<int>(val.engineModule + model_ambient_min, xorptr);
}

int main()
{
	int procID = MemClass.getProcess(L"csgo.exe");
	val.gameModule = MemClass.getModule(procID, L"client.dll");
	val.engineModule = MemClass.getModule(procID, L"engine.dll");

	val.localPlayer = MemClass.readMem<DWORD>(val.gameModule + dwLocalPlayer);

	if (val.localPlayer == NULL)
		while (val.localPlayer == NULL)
			val.localPlayer = MemClass.readMem<DWORD>(val.gameModule + dwLocalPlayer);

	SetBrightness();
	
	while (true)
	{
		if (GetKeyState(VK_NUMPAD0) & 1) {
			HandleGlow();
		}
		HandleRadar();

		Sleep(1);
	}
	return 0;
}