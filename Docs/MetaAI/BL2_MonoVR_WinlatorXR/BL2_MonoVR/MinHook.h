#pragma once
// Minimal MinHook compatible API for 32-bit x86
// Drop-in for BL2 - uses 5-byte JMP trampoline
#include <windows.h>

typedef enum MH_STATUS {
    MH_OK = 0,
    MH_ERROR_ALREADY_INITIALIZED,
    MH_ERROR_NOT_INITIALIZED,
    MH_ERROR_ALREADY_CREATED,
    MH_ERROR_NOT_CREATED,
    MH_ERROR_ENABLED,
    MH_ERROR_DISABLED,
    MH_ERROR_NOT_EXECUTABLE,
    MH_ERROR_UNSUPPORTED_FUNCTION,
    MH_ERROR_MEMORY_ALLOC,
    MH_ERROR_MEMORY_PROTECT,
    MH_ERROR_MODULE_NOT_FOUND,
    MH_ERROR_FUNCTION_NOT_FOUND
} MH_STATUS;

#ifdef __cplusplus
extern "C" {
#endif
MH_STATUS MH_Initialize(void);
MH_STATUS MH_Uninitialize(void);
MH_STATUS MH_CreateHook(LPVOID pTarget, LPVOID pDetour, LPVOID *ppOriginal);
MH_STATUS MH_EnableHook(LPVOID pTarget);
MH_STATUS MH_DisableHook(LPVOID pTarget);
MH_STATUS MH_RemoveHook(LPVOID pTarget);
#ifdef __cplusplus
}
#endif
