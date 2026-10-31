#pragma once
#include "pch.h"
#include "d3d9_proxy.h"

extern HMODULE g_hRealD3D9;
extern IDirect3D9* g_pRealD3D9;

// Global API instance
extern class WinXrApi* g_pWinXrApi;

// Simple trampoline hook for 32-bit - replace with MinHook for production
bool HookIAT(HMODULE mod, const char* dll, const char* func, void* newFunc, void** oldFunc);
bool HookVTable(void** vtable, int index, void* newFunc, void** oldFunc);

// Hooks for UE3
namespace Hooks {
    bool Init();
    void Shutdown();
    
    // WillowPlayerController::PlayerTick POST - continuous rotation applied each tick
    void OnPlayerTick(void* pc); // pc is WillowPlayerController*
    
    // WillowWeapon - for free gun aim
    void OnUpdateFirstPersonMesh(void* weapon);
}
