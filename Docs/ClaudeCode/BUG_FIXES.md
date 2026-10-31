# BUG_FIXES

Two parts. **Part 1** is an audit of the inherited scaffold in
`Docs/MetaAI/BL2_MonoVR_WinlatorXR/` - defects found by reading the code this
session, each with the fix. **Part 2** is the inherited bug log from
`Docs/MetaAI/BUG_FIXES.md`, reframed: those entries are written as fixes
already applied, but the code has never been compiled or run, so they are
**claims to verify**, not history.

Nothing here has been observed on hardware. **[code]** means found by reading
the file named. **[docs]** means it contradicts something
`Docs/general_wxr/` establishes.

---

## Part 1 - Defects in the inherited scaffold

Ordered by severity. File references are relative to
`Docs/MetaAI/BL2_MonoVR_WinlatorXR/BL2_MonoVR/`.

### 1. The proxy exports one of the four functions the game needs - **the game will not launch**

**[code]** `d3d9_proxy.cpp` exports only `Direct3DCreate9`. **[verified]** The
import table of `Borderlands2.exe` names four functions from `d3d9.dll`:
`Direct3DCreate9`, `D3DPERF_BeginEvent`, `D3DPERF_EndEvent`,
`D3DPERF_SetOptions`. All four are *static* imports, so the Windows loader
resolves them before the first instruction runs; a missing one is a hard
process-start failure with a dialog, not a degradation.

**Fix.** Export all four. The three `D3DPERF_` functions can be thin forwards
to the real DLL, or no-op stubs returning 0 - they are profiling hooks and the
game does not depend on their behaviour. A `.def` file is the least
error-prone way to get the names right with no decoration.

### 2. `DllMain` does socket setup, file I/O, thread creation and a 15-second wait

**[code]** `dllmain.cpp` calls `WinXrApi::Init()` from `DLL_PROCESS_ATTACH`,
and `WinXrApi::Init` (`WinXrApi.cpp`) calls `WSAStartup`, creates a socket,
writes files, starts a `std::thread` and then spins up to 150 x `Sleep(100)`
waiting for a first packet. All of that runs under the loader lock, in a
statically-imported DLL, before the process has an entry point.

Thread creation under the loader lock cannot complete - the new thread's own
`DLL_THREAD_ATTACH` notifications block on the lock the initialiser holds. The
15-second sleep then holds the lock for 15 seconds regardless.

**Fix.** `DllMain` does the bare minimum: stash the module handle,
`DisableThreadLibraryCalls`, open the log and write the build stamp. Everything
else happens on first `Direct3DCreate9`, which is called from the game's own
code with no lock held and is the natural "the game is really starting" signal.
The first-packet wait, if it is kept at all, belongs on the render thread with a
short timeout - never in loader init.

### 3. `MODE_VR = 2` is sent while being described as VR mode

**[code]** `d3d9_proxy.cpp` sends `SendHaptics(0,0,2,0,90.f,90.f)` - `MODE_VR=2`,
`MODE_3D=0` - and `WinXrApi.cpp`'s comment calls it "mono screen mode for 6DOF
without stereo buffers". `Docs/MetaAI/PROJECT_PLAN.md` repeats this as "`MODE_VR=2`
(screen mode that still sends 6DOF)".

**[docs]** That is the **non-VR** mode: a flat floating window in the headset,
with tracking still flowing. It is a legitimate and useful mode, but it is not
what a 6DOF camera port wants - and in it the runtime, not the mod, owns window
placement, so the mod must read bool 19 (`IS_IMMERSIVE`) and damp its tracking
scale by about 0.8 when the window is head-attached. None of that is done here.

**Fix.** For Mono VR send `MODE_VR=1`, `MODE_3D=0`. Reserve `2` for the non-VR
shipping fallback and, if that mode is shipped, implement the bool-19 branch
with it. Use `MODE_VR=0` for menus and pause, which stops the pose stream,
restores the runtime's own mouse and drops out of immersive presentation.

### 4. The outbound FOV is sent as `90.0 90.0`

**[code]** `d3d9_proxy.cpp`, the same `SendHaptics` call.

**[docs]** The outbound FOV fields are **degrees**, and the runtime applies
them only if greater than 1 - so 90 is at least in the right units, unlike the
tangent values some clients send. But 90 is not the value this stack was
developed against: both `opentrackWXR` and HWXR default to **104.5**, and that
is where to start. The field is a tuning value judged by eye through the
headset, not a conversion to get right.

**Fix.** Start at `104.5 104.5`. Make the pair loadable from a flag file so it
can be tuned on the device without a rebuild, with 104.5 as the fallback -
HWXR's pattern.

### 5. The sync pixel is drawn in `Present`, outside any scene

**[code]** `ProxyIDirect3DDevice9::Present` calls `DrawRedSyncPixel()`, which
calls `DrawPrimitiveUP` with no `BeginScene`/`EndScene` pair around it. Outside
a scene that call fails with `D3DERR_INVALIDCALL` and draws nothing. The return
value is discarded, so it fails silently.

It also draws into whatever render target is currently bound. In a UE3 frame
that is very likely not the back buffer - which is the defect the sibling
Interstellar Marines port's audit names as "the sync pixel is very likely being
drawn into an offscreen render target".

**Fix.** `PROJECT_PLAN.md` Feature 1 has the full sequence: bind the back
buffer explicitly, clear the depth-stencil binding, set the full viewport,
`BeginScene`, draw, `EndScene`, restore. And prove visibility first with an
oversized bright square rather than the near-black real sync value - a diagnostic
`ColorFill` on the back buffer rect is the cheapest version of that test
because it needs no scene and no state at all.

### 6. No quaternion is normalised

**[code]** `WinXrApiUDP::Parse` copies all three quaternions straight through.

**[docs]** The rotation shortcut is a rotation only when `|q| = 1`; a non-unit
quaternion also scales by `|q|^2`. Device packets are unit to about 1e-4, but
**tester packets are often 1.4% to 10.8% long**, which stretches each hand's
basis by a different amount and produces an asymmetry that looks like a deep
bug in the pose maths. This is a named, expensive bug from the Shogo port.

**Fix.** Normalise on ingest, unconditionally, with a zero-norm guard returning
identity.

### 7. The window is never made borderless, and never sized

**[code]** Nothing in the scaffold touches the window.

**[docs]** Three of the framework's non-negotiables depend on this. With chrome
in the corner the runtime samples the title bar instead of the sync pixel, and
head tracking silently degrades - which is "the single most common cause of *I
implemented sync and nothing changed*".

**Fix.** Strip `WS_CAPTION | WS_THICKFRAME | WS_BORDER`, size to the display
metrics, make it idempotent, and call it every frame from the `Present`
wrapper. UE3 rebuilds its window on mode changes and device loss. Watch for the
engine fighting back and resetting the mode when surface, render and client
sizes disagree.

### 8. COM reference counting on both proxy objects is wrong

**[code]** `ProxyIDirect3D9` and `ProxyIDirect3DDevice9` keep their own
`m_ref`, increment it in `AddRef` and `delete this` when the pre-decrement
reaches 0 - but neither is initialised to 1 at construction, and both return
the *real* object's count rather than their own. `ProxyIDirect3D9::QueryInterface`
also hands out the real interface, so a caller that queries for `IDirect3D9`
escapes the proxy entirely.

**Fix.** Initialise the proxy's own count to 1 in the constructor, return the
proxy's own count from `AddRef`/`Release`, and in `QueryInterface` return
`this` for the interfaces the proxy implements.

### 9. Haptics are dead code

**[code]** `SendHaptics` is only ever called with `0, 0`.

**[docs]** The first two outbound fields are stored **only when greater than
zero**, so sending 0 does not clear a previous rumble. They are a **duration in
frames**, not an amplitude. The working pattern is to decay a local value and
keep sending the decreasing number.

**Fix.** When haptics are implemented, decay locally and keep sending. This is
noted rather than urgent - the sibling Interstellar Marines audit records the
same defect ("haptics are dead code") in a port that is otherwise further along.

### 10. The hand-rolled trampoline hooker assumes a prologue

**[code]** `MinHook.cpp` is a 102-line 5-byte-JMP patcher, and
`Docs/MetaAI/BUG_FIXES.md` admits it "assume[s] push ebp/mov ebp,esp prologue".
A prologue that does not match, or an instruction boundary inside the first five
bytes, corrupts the function.

**Fix.** Do not hand-roll this. Use `bl-sdk/unrealsdk`, which already locates
and hooks UE3 functions for Borderlands 2 and is maintained. Where a native
detour is genuinely needed, use real MinHook as a submodule. Note
`unrealsdk`'s one-injection-per-process limit and build it `UNREALSDK_SHARED`.

### 11. The OpenXR-to-UE3 axis mapping has no sign flips and is almost certainly wrong

**[code]** `WinXrApiUDP::Parse` maps `(x, y, z)` to `(z, x, y) * 50`, with the
comment "simple conversion for prototype", and leaves all three quaternions
unconverted with a comment promising a 180-degree roll fix that is not applied.

XrApi is right-handed, +Y up, -Z forward; UE3 is left-handed, Z up, X forward.
Going between handednesses requires at least one sign flip, and this mapping has
none.

**Fix.** Rotate the canonical axes by the quaternion, *then* flip for
handedness - do not mirror the quaternion itself, which reliably produces a
basis that is subtly not orthonormal. Establish each axis's sign by moving the
headset along one axis at a time and reading a log line, not by argument. And
remember UE3's `FRotator` is int16-scaled, 65536 = 360 degrees.

### 12. The unit scale is asserted, not measured

**[code]** `const float UU_PER_M = 50.f;` with no derivation.

The UE3 nominal is 100 UU per metre (1 UU = 1 cm). 50 is very likely derived
from the classic Unreal character being 88 UU tall for a ~1.8 m human, which is
a character-proportion figure rather than a world-scale one. The two differ by a
factor of two; wrong gives either a camera glued still or a camera at the far
edge of the level.

**Fix.** Measure it. `PROJECT_PLAN.md` Feature 2 has the method. One named
constant, no bare multipliers anywhere else.

### 13. Smaller things

- **[code]** `WinXrApiUDP::Start` and `WinXrApi::Init` both call `WSAStartup`,
  and `Stop` and `Shutdown` both call `WSACleanup`. The counts happen to
  balance, but the pairing is per-component rather than per-process and will
  break the moment one is reordered. Do it once, at init.
- **[code]** `if(!(iss >> frameId)) frameId = (frameId+1) & 0xFF;` reads a
  value it has just failed to read. Harmless - `frameId` is 0 - but it means a
  malformed packet is accepted as frame 0 rather than rejected.
- **[code]** `XrPacket::R_pos[3] = {0,0}` initialises two of three elements.
- **[code]** The parser requires exactly 28 floats and 21 bool characters and
  returns false otherwise. **[docs]** The format gains fields at the end between
  versions, and the bool run can be 19 or 21 characters depending on how
  whitespace is treated. Tolerate trailing tokens and bounds-check every index.
- **[code]** `CreateStateBlock(D3DSBT_ALL, &sb)` runs every frame and the block
  is released every frame. Create it once and `Capture()` per frame.
- **[code]** The log is opened with `std::ofstream log("D:/BL2VR.log")` -
  truncating - in `DllMain` and appended elsewhere, at a hardcoded absolute
  path with no verbose gate. One logger, one path derived at runtime, build
  stamp on line 1, verbose parts behind a single flag that is off by default.
- **[code]** `Hooks::Init()` logs that it "would hook
  `WillowPlayerController::PlayerTick`" and returns true. Every game-side
  function in the scaffold is an empty stub. The game side does not exist.

---

## Part 2 - The inherited bug log, as claims to verify

`Docs/MetaAI/BUG_FIXES.md` reads as a record of fixes. The scaffold has never
been compiled, so none of it can be a record of anything observed. The entries
are still useful - several name real traps - but each needs a verdict.

| Inherited claim | Verdict |
|---|---|
| Locale parse failure on comma-decimal systems; fixed with `iss.imbue(std::locale::classic())` | **Sound and worth keeping.** The `imbue` call is present in the code **[code]**. A real trap, cheap insurance. |
| Red pixel culled in cinematics when drawn through the HUD Canvas; moved to the `Present` hook | **Half right.** The diagnosis is plausible and matches `Docs/general_wxr/` on draw ordering. The fix as implemented does not work - see Part 1 item 5. |
| `Z:` missing on desktop; fall back to `D:/xrtemp` | **Correct**, and `Docs/general_wxr/` endorses the pattern. But the code creates **both** directories and tests `Z:/` *after* creating them, and the newer reference clients derive the fallback from the current drive rather than hardcoding `D:`. Also: the runtime **wipes the handshake directory at startup**, so files must be written after it is up or rewritten. |
| FrameID wraps an 8-bit red channel; use `FrameID & 0xFF` | **Harmless but misconceived.** `HMD_SYNC` already arrives in 0-255 **[docs]**. The rule that matters is to **echo it exactly** - the runtime calibrates itself to whatever colour space the renderer produces, and the mapping is gated on green being 0. Do not rescale, pre-correct or invent ids. |
| Controller rotation rolled 180 degrees; add a roll correction | **Unverified, and the code does not implement it.** Do not pre-apply a correction nobody has observed. Establish it from a device log and a screenshot with controller overlays visible. Note the sibling port exposes a `flipHands` config flag for a "Pico/Quest 2 roll bug", so something in this area is real on some devices. |
| UU scale wrong, gun 2 m away; UE3 is ~50 UU per metre | **Not established.** See Part 1 item 12. |
| Game stalled on the loading screen with no UDP data; wait up to 15 s for the first packet | **The symptom is plausible; the fix is harmful.** A 15-second wait in `DllMain` is worse than the stall - see Part 1 item 2. The game must run with no headset present. Handle absence by freezing on a neutral pose and carrying on, not by blocking. |
| HMD altitude offset; use the 0.5 floor-relative `HMD_ALTITUDE` | **Cannot be confirmed.** `Docs/general_wxr/XRAPI_V0_4_DESC.md` documents 0.1 through 0.7 field by field from the runtime's own source and lists **no** `HMD_ALTITUDE` field; 0.5 is 0.4 plus the InputAPI listener, with no change to the tracking packet. Either this is a misremembering or the framework doc has a gap. **Resolve it from a device capture before relying on it.** |
| FEX runs out of RAM; cap the container at 4 GB, and the Debug DLL fails in the container | **Plausible and cheap to adopt.** Release|Win32 is the framework's standing advice anyway - a Debug CRT mismatch is a bad way to spend a day. |
| MinHook trampoline relocation crash | **Real limitation, wrong solution.** See Part 1 item 10. |
| Button mapping T/F parse | **Keep the parse, discard the table.** The button indices must be built from a device log of the raw bool string with the owner pressing each control in turn - the framework says to do exactly that and not to trust its own table. |

---

## How to use this file from here

Add an entry when something is **observed** - on the desktop with the tester, or
on device by the owner - with what was seen, what the cause turned out to be,
and how it was established. An entry for a fix that has not been run is a claim;
label it as one. The whole value of this file to a later session is that its
contents can be trusted.
