#include "pch.h"
#include "MinHook.h"
#include <map>
#include <vector>

struct HookInfo {
    LPVOID target;
    LPVOID detour;
    LPVOID original;
    BYTE originalBytes[5];
    BYTE trampoline[32];
    bool enabled = false;
};

static std::map<LPVOID, HookInfo> g_hooks;
static bool g_initialized = false;

MH_STATUS MH_Initialize(void) {
    if (g_initialized) return MH_ERROR_ALREADY_INITIALIZED;
    g_initialized = true;
    return MH_OK;
}

MH_STATUS MH_Uninitialize(void) {
    if (!g_initialized) return MH_ERROR_NOT_INITIALIZED;
    for (auto& kv : g_hooks) {
        MH_DisableHook(kv.first);
    }
    g_hooks.clear();
    g_initialized = false;
    return MH_OK;
}

MH_STATUS MH_CreateHook(LPVOID pTarget, LPVOID pDetour, LPVOID *ppOriginal) {
    if (!g_initialized) return MH_ERROR_NOT_INITIALIZED;
    if (g_hooks.find(pTarget) != g_hooks.end()) return MH_ERROR_ALREADY_CREATED;

    HookInfo info{};
    info.target = pTarget;
    info.detour = pDetour;

    // Save original 5 bytes
    memcpy(info.originalBytes, pTarget, 5);

    // Create trampoline: original 5 bytes + jmp back to target+5
    // For simplicity, we assume target starts with at least 5 bytes that can be relocated
    // This works for most UE3 functions that start with push ebp / mov ebp,esp
    DWORD oldProtect;
    VirtualProtect(info.trampoline, sizeof(info.trampoline), PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(info.trampoline, info.originalBytes, 5);
    // jmp [target+5]
    info.trampoline[5] = 0xE9;
    *(DWORD*)(info.trampoline+6) = (DWORD)((BYTE*)pTarget + 5 - (info.trampoline + 10));
    
    info.original = info.trampoline;
    info.enabled = false;

    if (ppOriginal) *ppOriginal = info.original;
    g_hooks[pTarget] = info;
    return MH_OK;
}

MH_STATUS MH_EnableHook(LPVOID pTarget) {
    if (!g_initialized) return MH_ERROR_NOT_INITIALIZED;
    auto it = g_hooks.find(pTarget);
    if (it == g_hooks.end()) return MH_ERROR_NOT_CREATED;
    if (it->second.enabled) return MH_ERROR_ENABLED;

    HookInfo& info = it->second;
    DWORD oldProtect;
    VirtualProtect(pTarget, 5, PAGE_EXECUTE_READWRITE, &oldProtect);
    // jmp detour
    BYTE* p = (BYTE*)pTarget;
    p[0] = 0xE9;
    *(DWORD*)(p+1) = (DWORD)((BYTE*)info.detour - (p + 5));
    VirtualProtect(pTarget, 5, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), pTarget, 5);
    info.enabled = true;
    return MH_OK;
}

MH_STATUS MH_DisableHook(LPVOID pTarget) {
    if (!g_initialized) return MH_ERROR_NOT_INITIALIZED;
    auto it = g_hooks.find(pTarget);
    if (it == g_hooks.end()) return MH_ERROR_NOT_CREATED;
    if (!it->second.enabled) return MH_ERROR_DISABLED;

    HookInfo& info = it->second;
    DWORD oldProtect;
    VirtualProtect(pTarget, 5, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(pTarget, info.originalBytes, 5);
    VirtualProtect(pTarget, 5, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), pTarget, 5);
    info.enabled = false;
    return MH_OK;
}

MH_STATUS MH_RemoveHook(LPVOID pTarget) {
    MH_DisableHook(pTarget);
    g_hooks.erase(pTarget);
    return MH_OK;
}
