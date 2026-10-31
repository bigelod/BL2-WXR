# CURRENT_PROGRESS.md - In-Progress Features

## What We Have Working / In Progress

### Project Scaffolding - In Progress
- VS2022 solution `BL2_MonoVR.sln` - Win32 Release, v143 toolset, outputs `d3d9.dll`
- File structure: `d3d9_proxy.h/cpp`, `WinXrApi.h/cpp`, `WinXrApiUDP.h/cpp`, `Hooks.h/cpp`, `MinHook.h/cpp`, `pch.h`
- MinHook minimal 5-byte JMP trampoline included (needs replacement with real MinHook for production)
- README + IDA guide + project docs

### WinlatorXR XrAPI 0.5 Handshake - Implemented
- Writes `Z:/tmp/xr/version=0.5` and `Z:/tmp/xr/vr=VR`, fallback `D:/xrtemp/` for desktop
- Creates directories if missing, deletes `vr` marker on shutdown
- Waits up to 15s for first packet like CoD4 mod

### UDP Protocol - Implemented (Desktop Tested with XrAPIUDPTester)
- Winsock2 listener on `127.0.0.1:7872`, non-blocking with 20ms timeout
- Parser for 28-float ASCII + FrameID + 21 bools (T/F), locale classic for '.' decimals
- Sender on `127.0.0.1:7278` with `<leftVib> <rightVib> <MODE_VR> <MODE_3D> <FOV>`
- Using `MODE_VR=2, MODE_3D=0` mono screen mode for 6DOF without stereo buffers
- Threaded recv with mutex-protected latest packet

### Red Sync Pixel - Implemented
- ProxyIDirect3DDevice9 wraps real device, hooks Present
- Draws 10x10 red block top-left `D3DCOLOR_ARGB(255, FrameID & 0xFF, 0, 0)` inside state block (restores game state)
- Updates last rendered FrameID from latest UDP packet before Present
- Based on HaloCEWXR and CoD4 pattern: packed backbuffer presented directly, no D3D11 copy

### Camera Hook - Stubbed, IDA Workflow Documented
- Hooks.h has `WillowPlayerController:PlayerTick` POST hook placeholder
- Logs first HMD pos to `D:/BL2VR.log`
- IDA Free workflow documented: find `WillowPlayerController`, `PlayerTick`, `PlayerCamera` via strings, use pattern `55 8B EC 83 EC ? 56 8B F1` etc
- Conversion: UU_PER_M = 50.f, OpenXR +Y up -Z forward -> UE3 Z-up
- Notes: Controller roll 180 deg fix needed, HMD_ALTITUDE floor-relative in 0.5

### Gun Hook - Stubbed
- `OnUpdateFirstPersonMesh` placeholder with comments for `SetAbsoluteTranslation(true)` and `SetTranslation(R_pos)`
- Plan to hook `GetMuzzleLocation` for free aim - not yet implemented

### Toolchain Integration - In Progress
- IDA Free guide for native x86 analysis, unrealsdk Python for quick prototyping, Cheat Engine for runtime float search
- dnSpy noted as not useful for native exe (only for C# tools like Gibbed)
- XrAPIUDPTester integration: enable sending UDP to localhost, use D:\xrtemp fallback

### FEX Setup - Documented, Not Yet Benchmarked
- Documented FEXCore .wcp location from K11MCH1/Winlator101 releases for Bionic Cmod v13+
- Container preset: Turnip + DXVK, Enable XR API, 1920x1080 mono to start
- Performance estimates from CoD4 (45-72 fps median 54 at 2388x1080) but not yet measured for BL2 + FEX
