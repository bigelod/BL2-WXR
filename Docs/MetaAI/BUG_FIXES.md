# BUG_FIXES.md - Short Bullet Log

- **Locale parse fail:** UDP parser failed on devices using comma decimals. Fix: `iss.imbue(std::locale::classic())` mandatory for '.' decimals - from container sends '.' decimals.
- **Red pixel disappears when HUD hidden:** Drawing pixel via HUD Canvas got culled in cinematics. Fix: Move to D3D Present hook inside BeginStateBlock/EndStateBlock capture so game D3D state restored afterwards.
- **Z: drive not found on desktop:** WinlatorXR marker `Z:/tmp/xr/` missing on PC. Fix: Fallback `D:/xrtemp/` when `Z:` absent, like HaloCEWXR fallback.
- **FrameID wrap:** Red channel only 8-bit, FrameID int wraps. Fix: Use `FrameID & 0xFF` for R channel, keep full int for pacing logic.
- **Controller roll 180 deg:** Gun appeared backwards. Fix: XrAPI controller rotation rolled 180 deg vs OpenXR aim pose, add extra quat roll correction.
- **UU scale wrong:** Gun 2m away. Fix: UE3 ~50 UU per meter, tune scale factor, apply relative to HMD not absolute world.
- **UDP timeout freeze:** Game stalled on loading HALO screen when no UDP data. Fix: Wait up to 15s for first packet, block InputHandler::UpdateInputs until fresh tracking arrives (OpenXRFrameWait pattern from HaloCEWXR).
- **HMD altitude offset:** Camera too high/low, crouch stuck. Fix: Use HMD_ALTITUDE floor-relative from 0.5, sync player relative to game not just device tracking.
- **FEX RAM crash:** FEX-Emu/Steam needs more RAM than tablet physically has on Quest. Fix: Limit container RAM to 4GB, use Release build not Debug for d3d9.dll (Debug DLL failed on WinlatorXR container).
- **MinHook trampoline relocation:** Original 5 bytes not relocatable caused crash. Fix: For now assume push ebp/mov ebp,esp prologue, TODO replace with real MinHook or use unrealsdk hook for WillowPlayerController:PlayerTick POST.
- **Button mapping T/F parse:** No bools in packet caused false positives. Fix: Parse 21 bools as T/F chars, ignore HMD name string at end (extraction commented out in WinXrApi.cpp:569).
