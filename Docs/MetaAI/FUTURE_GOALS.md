# FUTURE_GOALS.md - Remaining Features (Not Yet Started)

## Core VR Features - Not Started
- **True Stereo Modes:** Implement SBS (MODE_3D=1) and AER (MODE_3D=2) after mono stable - render eye 0 blit, eye 1 blit, keep red pixel with blue channel = RenderEyeValue. Benchmark vs mono to see if FEX can sustain 60fps stereo.
- **Independent Gun Aim Logic:** Hook `WillowWeapon::GetMuzzleLocation` + `CalcWeaponFire` to use R controller forward instead of camera forward. Add optional two-handed aim (L controller stabilizes).
- **Off-hand / Free Glove:** Palm pose derived from corrected aim, rolled by `OFFHAND_ROLL` like CoD4 mod does. Show left hand ghost for reload.
- **Melee Gesture:** Physical swing detection from controller velocity (estimate from successive UDP packets, like WinlatorXR CoD4 does). Map to melee key.
- **Grenade Belt:** Frag/tactical on hips from L/R grip like CoD4 - reach to belt to equip.
- **Scope / PiP:** Functional picture-in-picture scope for snipers, not just zoom FOV.

## Input & Haptics
- **Full Button Remap:** L_GRIP sprint, R_GRIP reload, L_TRIGGER ADS, R_TRIGGER fire, thumbstick press crouch, R stick snap turn 30 deg, Y/X for action skill / grenade. Make config file `BL2VR.ini`.
- **Haptics:** Map weapon fire to `SendHaptics(rightVib)` on 7278, different amplitudes per weapon type like HaloCEWXR `PlasmaPistolHapticStrength` etc.
- **Velocity-based Throw:** Estimate controller velocity from packets for grenade throw strength.

## HUD & Comfort
- **HUD Reprojection:** Move HUD from head-locked to world-anchored screen 2m in front, fixed in room when appears (like CoD4 does for menus/cinematics). Disable head bob, dynamic shadows.
- **Recenter:** Hold Menu to recenter HMD origin relative to game, like HWXR recentre code.
- **Seated Non-VR Mode:** Optional `MODE_VR=2` toggle - play like regular game with added 6DOF tracking (requested feature in HWXR).
- **Crosshair Options:** Floating crosshair vs gun-attached dot.

## Performance & Compatibility
- **FEX Preset Tuning:** Test FEXCore nightly 2609 builds vs Box64 Performance preset, document FPS at 1920x1080 vs 2388x1080 vs 2864x1296.
- **DXVK / Turnip Tuning:** Test DXVK 1.10.1 vs latest, Turnip driver versions, audio quality LOW for perf.
- **Co-op Verification:** Ensure Willow network code not broken by camera detach - test host/client with 2 Quest devices.
- **DLC & Mods:** Verify all DLC (Tiny Tina, etc) and BL2 community patch work with d3d9 proxy.

## Platform & Distribution
- **WinlatorXR Shortcut:** Create `Launch-BL2-VR-WinlatorXR.bat` + desktop shortcut like CoD4/HaloCEWXR do.
- **Installer:** Bundle vcruntime140, msvcp140, ws2_32 etc if needed for System32/SysWow64 (like HaloCEWXR README).
- **Reshade SuperDepth:** Experiment with Reshade to inject fake depth into mono for cheap stereo.
- **Left/Right Hand Swap:** Config for southpaw.
