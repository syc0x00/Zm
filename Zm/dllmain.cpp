// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "Zm.h"
#include "Hooks.h"
#include "Helpers.h"
#include "Math.h"
#include "Offsets.h"
#include "Game.h"
#include "Menu.h"

// My first ever other than an external one for a 2d game where I wrote some mem manip funcs including AOB in C#
// so please no bulli
// also i stole some funcs (w2s and some helpers) from online and i probably didnt credit every1 sry 

HMODULE g_DLLMOD;

void Input() { // Have NumLock on
	Console::Log("Input started");

	Vector3 oldPos;
	SHORT keystate; // while will assume int32 otherwise because cpp

	while (1) {
		if ((keystate = GetKeyState(VK_F9) & 0x8000)) { // F9 teleport to nearest ent on cursur
			Offsets::gentity_t *self = GetGentity(0);
			POINT curs;
			GetCursorPos(&curs);
			int closeID = 0;
			POINT closest = {0xFFFF, 0xFFFF}; // I hope no one has a display over 65535 pixels

			float viewMatrix[16];
			int v = 0;
			for (int i = 0; i < 64; i = (i + 0x4)) {
				float *base = reinterpret_cast<float *>(0x033F0400 + i);
				viewMatrix[v++] = *base;
			}

			for (int i = 20; i < 100; i++) { // Ignore ourselves with the starting 1
				Offsets::gentity_t *ent = GetGentity(i);
				if (ent->Health > 0 && ent->Type == EntityType::ZOMBIE) {
					Vector3 screen;
					Vector3 pos;
					pos.x = ent->Position[0];
					pos.y = ent->Position[1];
					pos.z = ent->Position[2];

					if (Math::WorldToScreen(pos, screen, viewMatrix)) {
						if (curs.x - screen.x < closest.x) {
							closest.x = curs.x - screen.x;
							closeID = ent->ClientNum;
						}
					}
				}
			}

			Offsets::gentity_t *ent = GetGentity(closeID);

			oldPos.x = Offsets::Pos->x;
			oldPos.y = Offsets::Pos->y;
			oldPos.z = Offsets::Pos->z;
			Offsets::Pos->x = ent->Position[0];
			Offsets::Pos->y = ent->Position[1];
			Offsets::Pos->z = ent->Position[2];
			Console::Log("Teleported");
		}
		if ((keystate = GetKeyState(VK_F8) & 0x8000)) { // Go back to old position
			Offsets::Pos->x = oldPos.x;
			Offsets::Pos->y = oldPos.y;
			Offsets::Pos->z = oldPos.z;
		}
		if ((keystate = GetKeyState(VK_F7) & 0x8000)) { // Register tag j_head
			int ret = Game::RegisterTag("j_head");
			Console::Value("ret", ret);
		}
		if ((keystate = GetKeyState(VK_F6) & 0x8000)) { // Get client wep1 id
			Offsets::gentity_t *self = GetGentity(0);
			Console::Value("self->PlayerInfo->Weapon1ID", self->PlayerInfo->Weapon1ID);
		}
		if ((keystate = GetKeyState(VK_F4) & 0x8000)) { // Set all zombies health to 1 = 1 hit die
			Console::Log("F4: Omae wa mou shindeiru");
			for (int i = 2; i < 1024; i++) {
				Offsets::gentity_t *ent = GetGentity(i);
				if (ent->Type == EntityType::ZOMBIE) {
					ent->Health = 1;
				}
			}
		}

		Sleep(50);
	}
}

void Main() {
	Helpers::Init();

	//Zm::Dump(); // Dump to test it's working

	HANDLE D3Dhandle = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Hooks::Init, NULL, NULL, NULL);
	Sleep(10);
	HANDLE TPHandle = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Input, NULL, NULL, NULL);

	WaitForSingleObject(D3Dhandle, INFINITE); // Wait for the F10 which will undo the Hooks
	TerminateThread(TPHandle, 0); // Then remove the input thread

	CloseHandle(TPHandle); // Close the handles
	CloseHandle(D3Dhandle);

	Console::Log("Successful unhook and TerminateThread()");

	Helpers::Exit(0); // and detach
	return;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	g_DLLMOD = hModule;
	DWORD ThreadID;

    switch (ul_reason_for_call) {
		case DLL_PROCESS_ATTACH:
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Main, NULL, NULL, &ThreadID);
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}
