#include "pch.h"
#include "d3d9_proxy.h"
#include "WinXrApi.h"
#include "Hooks.h"

HMODULE g_hModule = nullptr;
WinXrApi* g_pWinXrApiLocal = nullptr;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        g_hModule = hModule;
        DisableThreadLibraryCalls(hModule);

        // Init WinlatorXR API - writes Z:/tmp/xr/version 0.5 and Z:/tmp/xr/vr=VR
        g_pWinXrApiLocal = new WinXrApi();
        g_pWinXrApi = g_pWinXrApiLocal;
        
        if (g_pWinXrApiLocal->Init()) {
            // Log success
            std::ofstream log("D:/BL2VR.log");
            log << "[BL2_MonoVR] WinXrApi init OK, FEX path active\n";
        } else {
            std::ofstream log("D:/BL2VR.log");
            log << "[BL2_MonoVR] WinXrApi init failed, running without VR\n";
        }

        // Init hooks for WillowPlayerController etc
        Hooks::Init();
    }
    else if (reason == DLL_PROCESS_DETACH) {
        Hooks::Shutdown();
        if (g_pWinXrApiLocal) {
            g_pWinXrApiLocal->Shutdown();
            delete g_pWinXrApiLocal;
            g_pWinXrApiLocal = nullptr;
            g_pWinXrApi = nullptr;
        }
    }
    return TRUE;
}
