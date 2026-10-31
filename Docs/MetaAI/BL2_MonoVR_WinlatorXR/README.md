# BL2_MonoVR - Borderlands 2 Flat -> Monocular 6DOF WinlatorXR (FEX)

VS2022 x86 DLL project for Borderlands 2 (32-bit) -> WinlatorXR XrAPI 0.5 mono 6DOF.

This is the starter for what we discussed: no stereo eye buffers, WinlatorXR does HMD compositing, you just feed HMD pose to camera and R controller pose to gun.

## What it does
- Writes `Z:\tmp\xr\version=0.5` and `Z:\tmp\xr\vr=VR` (fallback `D:\xrtemp\` for desktop test)
- UDP listener 127.0.0.1:7872 parses 28 floats + FrameID + 21 bools (OpenXR +Y up, -Z forward, meters)
- UDP sender 127.0.0.1:7278 sends `<leftVib> <rightVib> <MODE_VR> <MODE_3D> <FOV_W> <FOV_H>` with MODE_VR=2 (screen) + MODE_3D=0 (mono)
- d3d9.dll proxy - intercepts Present and draws 10x10 red sync pixel (FrameID % 256) top-left
- Hooks WillowPlayerController:PlayerTick POST for HMD->Camera
- Detaches FirstPersonMesh to R controller for free gun aim
- Maps XrAPI buttons to InputKey

## Build
- VS2022 with v143 toolset, Windows 10 SDK
- Platform: Win32 (Borderlands 2 is 32-bit), Release
- Output: `d3d9.dll` - drop next to `Borderlands2.exe`
- Requires: `d3d9.lib`, `ws2_32.lib`
- For full ProcessEvent hooks, add MinHook (included as simple trampoline here, replace with real MinHook if you want)

## FEX + WinlatorXR setup
- Use Winlator Bionic Cmod v13+ with FEXCore .wcp (from K11MCH1/Winlator101 releases)
- Container: Turnip + DXVK, Enable XR API, Virtual screen 1920x1080 mono
- Put game in D: -> Download/Borderlands2
- Copy built d3d9.dll next to exe

## Flow
1. Build, run desktop test - should create D:\xrtemp\vr
2. Deploy to Quest, check Z:\tmp\xr\system exists
3. Enable logging in `WinXrApi.cpp` -> `D:\BL2VR.log`
4. First test: camera only, gun still attached
5. Then enable gun detach in GunHook.cpp

See source comments for coordinate conversion (50 UU per meter) and 180 deg roll fix for controllers.
