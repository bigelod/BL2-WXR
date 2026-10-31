# IDA Free + Borderlands 2 (UE3) - Finding offsets for Mono 6DOF

BL2 is 32-bit Unreal Engine 3, not .NET, so **IDA Free is gold, dnSpy is not useful** here. dnSpy is for .NET assemblies - BL2.exe is native C++.

### IDA Free workflow

1. Open `Borderlands2.exe` in IDA Free 8.4+ (x86). Let auto-analysis finish.

2. **Find GObjects / GNames** - UE3 globals:
   - Strings -> search `GObjects`, `GNames`, `WillowGame`, `WillowPlayerController`
   - In BL2, GObjects is usually at `Borderlands2.exe+0xXXXXX` - pattern: `A1 ? ? ? ? 8B ? ? 8B ? ? 25 ? ? ? ? 74 ?`
   - You can also use existing UE3 SDK dumper: https://github.com/Archaines/bl2-sdk or unrealsdk already dumps these

3. **WillowPlayerController::PlayerTick**
   - In Strings window, search `PlayerTick`
   - Xref to `WillowGame.WillowPlayerController.PlayerTick` - you'll see a function that calls `PlayerInput`, `UpdateRotation`
   - The POST hook you want is right after the original tick - in unrealsdk you just do:
     ```python
     from mods_base import hook
     @hook("WillowGame.WillowPlayerController:PlayerTick", Type.POST)
     def my_tick(obj, args, ret, func):
         # obj is WillowPlayerController*
         pass
     ```
   - In C++ d3d9.dll, find its address: In IDA, look for `E8 ? ? ? ? 8B 4E ? 8B 11` near PlayerTick string. That call is to ProcessEvent.

4. **Camera**
   - Search `PlayerCamera` in strings
   - `WillowPlayerController` has `PlayerCamera` property at offset ~0x2AC (varies by version)
   - Camera class `WillowPlayerCamera` or `GamePlayerCamera` - function `UpdateViewTarget`
   - For mono 6DOF, you want to override `Location` and `Rotation` in PlayerTick, before camera update

5. **FirstPersonMesh / Weapon**
   - Search `FirstPersonMesh`, `WeaponMesh`, `WillowWeapon`
   - Function `WillowWeapon::UpdateFirstPersonMesh` or `AttachWeaponToPlayer`
   - In IDA: look for `FirstPersonMesh` string, xref to function that does `SetTranslation` on mesh
   - That's where you detach: `SetAbsoluteTranslation(true)` then `SetTranslation(R_pos)`

6. **Pattern scan for release builds**
   Since BL2 updates break offsets, use sigscan instead of hard offsets:
   ```
   PlayerTick: 55 8B EC 83 EC ? 56 8B F1 8B 06 8B 90 ? ? ? ? FF D2
   GetMuzzleLocation: 55 8B EC 83 EC 20 56 8B F1 E8 ? ? ? ? 8B 4E 18
   ```

### dnSpy - when is it useful?

Only if you're looking at **unrealsdk Python mods** or **Borderlands 2 SDK Python stubs** - those are Python, not .NET. For C# tools like WillowTree or Gibbed, dnSpy helps, but not for the exe itself.

Use **Cheat Engine** + unrealsdk for runtime:
- CE: Attach to Borderlands2.exe, search float for camera X, move head in your VR test, see what changes
- unrealsdk console: `obj dump WillowPlayerController` -> shows all properties and offsets live

### Quick verification with IDA Free

After you hook PlayerTick, log to `D:/BL2VR.log`:

```
[Hooks] PlayerTick at 0x12345678
HMD pos: 0.1 1.6 -0.2 FrameID: 42
```

If you see HMD pos updating from WinlatorXR UDP 7872, your FEX path is working. Then enable gun detach.

### Recommended tool order

1. **unrealsdk** (Python) - fastest to prototype camera = HMD without C++
2. **IDA Free** - find exact function addresses for C++ d3d9.dll hook
3. **Cheat Engine** - verify mesh detachment at runtime
4. **VS2022 + MinHook** (now included in project) - final d3d9.dll

The project now includes MinHook.h/cpp - no external dependency. Just build Release Win32 and copy d3d9.dll next to exe. For desktop test, create `D:\xrtemp\vr` and `version` files and send fake UDP packets from Python to test red pixel.
