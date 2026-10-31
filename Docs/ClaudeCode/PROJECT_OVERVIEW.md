# BL2-WXR - project overview

**Goal.** A playable **monocular 6DOF VR** experience of **flat, non-VR
Borderlands 2** on a standalone headset (Quest 3 first), through
**WinlatorXR** and **XrApi 0.5**.

Written 31 Oct 2026. Nothing in this document has been tested on hardware.

**Evidence tags.** **[verified]** = checked on this machine, this session, and
the check is named. **[docs]** = from `Docs/general_wxr/`, which is itself
verified against a Quest 3 and the WinlatorXR source. **[inherited]** = from
`Docs/MetaAI/`, an earlier planning pass, *unverified*. **[web]** = from a web
source named in place. **[unverified]** = a belief that needs an experiment
before anything is built on it. Anything untagged is reasoning, not fact.

---

## 1. What we are making, in one paragraph

Borderlands 2 keeps its own renderer and runs as it always has: one flat
window, one camera, one view per frame. A mod inside the process takes the
headset's head pose off a UDP socket and writes it into the game's camera, so
looking around and leaning work in all six degrees of freedom. It takes the
dominant controller's pose and points the weapon with it. It draws one small
red square in the corner of the frame so WinlatorXR can match each image to the
pose it was rendered for. WinlatorXR shows that single image to both eyes.

There is no stereo at the start, and possibly never. That is a deliberate
choice, not a shortfall - see section 4.

---

## 2. Why flat BL2 and not Borderlands 2 VR

Borderlands 2 VR is a separate product with real VR support, and the obvious
question is why not just run it through `OXRWXR` + `OpenComposite`
(`Docs/general_wxr/WINLATORXR_DESC.md`, Path E). The answer is that the
official VR build trades away most of what makes Borderlands 2 worth playing:

| | Flat BL2 + this mod | Borderlands 2 VR via OpenComposite |
|---|---|---|
| Co-op | **the game's own networking, untouched** | single-player only **[web]** |
| DLC | all 24 DLC folders present on this install **[verified]** | the BAMF pack only |
| Community mods, BLCMM, patches | work as normal | no |
| Translation layers | Box64/FEX + Wine + DXVK | the same, **plus** OpenComposite and OXRWXR |
| Built for | a 2012 desktop CPU | a desktop **VR** GPU |
| Presentation | ours to choose | whatever the stack gives us |

The second column pays translation overhead on content that was already too
expensive, which is the specific failure mode
`Docs/general_wxr/WINLATORXR_DESC.md` warns about. The owner also reports the
official port's stereo presentation behaving poorly in this stack - an
observation worth confirming, and recorded as an intake question in
`PROJECT_PLAN.md` rather than assumed here.

So: **mod the flat game directly.** That is also the standing recommendation of
`Docs/general_wxr/CLAUDE.md` and needs no further justification.

---

## 3. What we know about the target

All **[verified]** on this machine against
`E:\Steam\steamapps\common\Borderlands 2`:

| Fact | Value | How it was checked |
|---|---|---|
| Executable | `Binaries/Win32/Borderlands2.exe`, 27.4 MB | directory listing |
| Bitness | **PE32 / x86** (machine `0x14c`, optional-header magic `0x10b`) | PE header parse |
| Renderer | **Direct3D 9** - statically imports `d3d9.dll` and `d3dx9_43.dll` | import-table parse |
| `d3d9.dll` exports actually used | **only four**: `Direct3DCreate9`, `D3DPERF_BeginEvent`, `D3DPERF_EndEvent`, `D3DPERF_SetOptions` | import-table parse |
| Input | `DINPUT8.dll` (`DirectInput8Create`) and `XINPUT1_3.dll` (ordinals 2, 3) | import-table parse |
| Engine | Unreal Engine 3, `WillowGame` | `WillowGame/CookedPCConsole`, `Engine/Config/BaseInput.ini` |
| Changelist | `2863302` | `changelist.txt` |
| DLC installed | 24 folders under `DLC/` | directory listing |
| Default resolution | `ResX=1280 ResY=720`, `Fullscreen=true` | `WillowGame/Config/DefaultEngine.ini:1150` |
| Aspect handling | `AspectRatioAxisConstraint=AspectRatio_MaintainYFOV` | `DefaultEngine.ini:1372` |
| Auto graphics detection | `bAutoDetectSettings=true` | `DefaultEngine.ini`, `[SystemSettings]` |
| Console key | `ConsoleKey=Undefine` by default | `Engine/Config/BaseInput.ini:229` |

Three of these change the plan on their own:

- **Four exports.** A `d3d9.dll` proxy is a *tiny* amount of code here - four
  entry points, not the hundred-odd a generic proxy carries. And because the
  import is static, the proxy loads before the game's first instruction.
- **`MaintainYFOV`.** Forcing a square resolution like 1400x1400 keeps the
  *vertical* FOV and narrows the horizontal one, rather than letterboxing. That
  is the good failure mode, and it means square modes are worth trying early
  (`Docs/general_wxr/WINLATORXR_DESC.md`: "square-ish resolutions suit
  headsets").
- **`bAutoDetectSettings=true`.** This is exactly the "games of this era
  silently restore graphics settings" trap named in
  `Docs/general_wxr/FIRST_STEPS.md`. Turn it off in stage 1, then check it
  stayed off.

---

## 4. The honest performance position

**This is the heaviest game anyone in this family of ports has attempted, and
it is the project's main risk.**

`Docs/general_wxr/WINLATORXR_DESC.md` is explicit: "Old games are the sweet
spot... a 1998 renderer has so much headroom on a Quest 3 that 2.25x
supersampling costs little", and "a 2007 engine may not leave enough for stereo
at all". Borderlands 2 is **2012**, Unreal Engine 3, and it is reported to be
**CPU-bound even on desktop hardware** **[web]** - which is the worst possible
shape for a stack whose first layer is x86 instruction translation. No
benchmark of Borderlands 2 under Winlator on a Quest 3 appears to exist; a web
search for one returned nothing.

What follows from that:

1. **Mono is the target, not a stepping stone.** `MODE_VR = 1`, `MODE_3D = 0`.
   One render per frame, full per-eye frame rate, no depth. Shipping mono is
   what the Tremulous port does and it is a legitimate product **[docs]**.
2. **AER is the only plausible route to depth** (`MODE_3D = 2`: one eye per
   frame, alternating - real stereo at mono scene cost). **SBS is very
   unlikely to be affordable** and should not be planned for.
3. **Non-VR mode stays on the table as a shipping fallback** (`MODE_VR = 2`): a
   flat window in the headset with tracking still live, driving the weapon but
   not the camera. It needs no camera work, cannot induce vection, and costs
   the least. If the camera turns out to be unreachable, this is still a
   product.
4. **Measure before promising anything.** Stage 1 of the plan is "does the
   unmodified game run in a container at a playable frame rate" and it is a
   genuine go/no-go gate, not a formality. Performance is measured on-device
   with WinlatorXR's own counters, never by instrumenting the mod
   (`Docs/general_wxr/CLAUDE.md`).

---

## 5. The stack

```
  Quest 3 (Android, arm64)
  +-- WinlatorXR
      +-- Box64 or FEX          x86 -> arm64 translation   <- the CPU wall
      +-- Wine + container      virtual display, e.g. 1400x1400 or 2800x1400
      |   +-- DXVK              D3D9 -> Vulkan
      |       +-- Borderlands2.exe   (32-bit, UE3, WillowGame)
      |           +-- our d3d9.dll   XrApi client, sync pixel, window,
      |                              and the UE3 camera / weapon / input hooks
      +-- XR layer              captures the window, warps, presents
                                 UDP 7872 -> poses in
                                 UDP 7278 <- haptics, modes, FOV
                                 red corner pixel <- which frame this is
```

Nothing of ours runs on Android. It is one 32-bit Windows DLL speaking Win32, a
UDP socket and a few files. **[docs]**

---

## 6. The target experience

A first-person Borderlands 2 played with your head and hands:

- **Head** drives the camera in 6DOF - look, lean, crouch by crouching.
- **Dominant hand** aims the gun. The weapon model follows the controller; the
  shot is still fired by the game's own weapon code.
- **Off hand** does movement and the utility actions.
- **Co-op survives**, because the game's networking and firing logic are never
  replaced - only the camera and the aim vector are written.
- **One flat image**, shown to both eyes, with correct frame-to-pose
  correlation so it feels locked rather than swimmy.

Tiers of done, in order:

| Tier | What the player gets |
|---|---|
| **A** | 6DOF head tracking in mono. Looking around and leaning feel locked. Gun still on the camera. |
| **B** | A full VR control scheme: gun on the dominant hand, free aim, sticks, grips, haptics. |
| **C** | Comfort and polish: HUD scale for square resolutions, recentre, snap turn, an in-game settings menu. |
| **D** | Optional depth via AER, if and only if it measures as affordable. |

**Tier A is the project.** B, C and D are what makes it good.

---

## 7. Non-goals

Taken from `Docs/general_wxr/CLAUDE.md` and not to be relitigated without
asking the owner:

- **No renderer modernisation.** No DX11, no render-to-texture rewrite. The
  game's D3D9 path already works under DXVK, which is the path the whole stack
  is tuned for.
- **No 64-bit build.** There is no OpenXR runtime to load, which is the only
  reason other VR mods go 64-bit. Mixed bitness would force IPC, and the IPC
  would become the biggest source of bugs in the project.
- **No wrapper layers** - no dgVoodoo, no ReShade shipped by us. WinlatorXR can
  inject ReShade itself if a player wants to experiment.
- **No XrApi 0.6 or 0.7.** 0.5 is human-readable in a log and the UDP Tester
  can simulate it. A higher version number is a cost, not a goal.
- **No frame timing built into the mod.** It cannot see the compositor, the
  translation layers or the GPU, which is where the time goes.
- **No in-repo copies of the game's content, source or cooked packages.**

---

## 8. What is not yet decided, and needs the owner

These are intake questions. They are recorded as `TBD` in `PROJECT_PLAN.md`
section 1 and must not be answered by guessing.

1. Does the unmodified game **launch at all** in a WinlatorXR container, and
   does it need Steam running inside the container (store mode) to do it?
2. What frame rate does it manage flat, at which container resolution, on Box64
   versus FEX?
3. Is **co-op** in scope as a tested feature, or merely "not deliberately
   broken"?
4. What exactly was wrong with the official Borderlands 2 VR build's
   presentation in this stack - the owner's AER observation?
5. Right-handed, left-handed, or both from the start?
6. Is the result to be published, and under what licence?

---

## 9. Reading order

1. `Docs/general_wxr/` - the framework. `README.md` has the map.
2. This file.
3. `Docs/ClaudeCode/PROJECT_PLAN.md` - the intake table and the staged plan.
4. `Docs/ClaudeCode/CURRENT_STATUS.md` - where the project actually is.
5. `Docs/ClaudeCode/BUG_FIXES.md` - defects found in the inherited scaffold,
   and the inherited bug log reframed as claims to verify.
6. `Docs/MetaAI/` - the first planning pass. Useful, and wrong in places that
   `BUG_FIXES.md` names.
