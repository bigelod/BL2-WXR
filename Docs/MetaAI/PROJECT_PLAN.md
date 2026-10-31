# PROJECT_PLAN.md - BL2 Mono 6DOF VR for WinlatorXR

## Goal
Give standalone VR players (Quest 2/3/Pro, Pico 4) a VR experience of **Borderlands 2** with 6DOF head and gun tracking, prioritizing raw performance over stereo 3D.

Target stack: **WinlatorXR + XrAPI 0.5 (UDP-based) + FEXCore + Borderlands 2 (flat, non-VR)** in monocular 6DOF mode.

## Why This Stack

**WinlatorXR XrAPI 0.5 Protocol:**
- Marker files: `Z:\tmp\xr\version=0.5` and `Z:\tmp\xr\vr=VR` (fallback `D:/xrtemp/` for desktop testing)
- UDP listen `127.0.0.1:7872` - 28-float ASCII packet: L/R/HMD quats (4+4+4), positions (3+3+3), thumbsticks (2+2), IPD, FOV, FrameID, 21 bools (button states)
- UDP reply `127.0.0.1:7278` - `<leftVib> <rightVib> <MODE_VR> <MODE_3D> <FOV_W> <FOV_H>`
- Poses: OpenXR convention +Y up, -Z forward, meters, with HMD_ALTITUDE floor-relative in 0.5
- Red sync pixel 10x10 top-left before Present, `D3DCOLOR_ARGB(255, FrameID & 0xFF, 0, 0)` - load-bearing for frame pacing

**Monocular Mode:**
- `MODE_3D=0`, `MODE_VR=2` (screen mode that still sends 6DOF). Game renders single window, WinlatorXR composites to HMD. No eye buffers = ~1.8-2.2x fps vs stereo.
- Reference: CoD4 WinlatorXR 45-72 fps median 54 at 2388x1080 limited by Box64/DXVK, drops to ~35 at 2864x1296.

**Why Flat BL2, Not BL2 VR Branch:**
- Option A (flat BL2 -> VR): Keep all DLC, co-op, mods, community patches, better CPU perf. BL2 is native 32-bit D3D9 UE3 (WillowGame).
- Option B (BL2 VR -> XrAPI): Would require stripping OpenVR, but still has VR branch overhead and breaks co-op.
- Decision: **Flat BL2 (non-VR original) modding for monocular 6DOF VR on FEX is best** for performance.

**Why FEX, Not Box64:**
- WinlatorXR supports Box64, Turnip, FEX. FEXCore expected 15-30% faster CPU translation than Box64 for UE3 (estimate, not yet benchmarked on BL2), but more RAM usage. Critical for Quest 3.

## Architecture

```
Quest Headset (OpenXR) 
  -> WinlatorXR Runtime (scrapes HMD + controllers)
  -> UDP 7872 -> d3d9.dll proxy (our mod) in Borderlands2.exe
  -> Hooks: WillowPlayerController:PlayerTick POST for camera = HMD (scale ~50 UU per meter)
          FirstPersonMesh detach to R controller pose
          GetMuzzleLocation / GetAdjustedAim for free aim
          InputKey mapping from 21 bools
  -> UDP 7278 reply with haptics + MODE_VR=2 + red pixel
  -> WinlatorXR composites mono window to HMD
```

**Proxy d3d9.dll:** Next to Borderlands2.exe, loads real system d3d9, forwards exports, hooks IDirect3DDevice9::Present for red pixel and hooks UE3 via MinHook / vtable.

## Build Order
1. Handshake + UDP + red pixel + HMD->camera (head 6DOF)
2. Gun mesh = R controller (right hand 6DOF)
3. Aim override + input + haptics

## Desktop Testing
Use XrAPIUDPTester (https://github.com/WinlatorXR/XrAPIUDPTester) - Enable sending UDP to localhost, create D:\xrtemp\vr and version files, run BL2 with dll, check D:/BL2VR.log and red pixel. No headset needed until gun detach.

## Toolchain
IDA Free for native x86 analysis (WillowPlayerController, PlayerTick, FirstPersonMesh), Cheat Engine for runtime float search, unrealsdk Python for quick prototyping, VS2022 Release Win32 for final d3d9.dll, FEXCore .wcp for Quest deployment.
