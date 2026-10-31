# PROJECT_PLAN - Borderlands 2 to standalone VR via WinlatorXR

Filled from the template in `Docs/general_wxr/PROJECT_PLAN.md`. Evidence tags
as defined in `PROJECT_OVERVIEW.md` section 0.

> **Sections 1 and 2 are an intake interview and are not finished.** Every
> `TBD` below is a question only the owner, the device or an experiment can
> answer. A visible `TBD` is a prompt; a guess would be a bug. Do not start
> stage 2 until section 1's "The port" block is filled in.

---

## 1. Intake

### The game

| Question | Answer |
|---|---|
| Game, version, patch level | Borderlands 2, Steam, changelist `2863302` **[verified]** |
| Store or distribution | Steam (app id 49520, `steam_appid.txt` present) **[verified]** |
| Install path | `E:\Steam\steamapps\common\Borderlands 2` **[verified]** |
| Bitness | **32-bit** (PE32, machine `0x14c`) **[verified]** |
| Renderer | **Direct3D 9**; imports `d3d9.dll` (4 functions) + `d3dx9_43.dll` **[verified]** |
| Engine | Unreal Engine 3, Gearbox `WillowGame` **[verified]** |
| Source available? | No. No SDK release, no GPL tree. |
| Mod API or SDK? | **Yes, two.** `bl-sdk/willow2-mod-manager` (Python mods via `pyunrealsdk`) and `bl-sdk/unrealsdk` (a C++20 library for UE3 object/hook access, BL2 listed as supported). Both reach the game through a `ddraw.dll` the game loads. **[web]** |
| Does a VR build already exist? | Yes - **Borderlands 2 VR**, a separate Steam product, single-player only. Rejected as the base; see `PROJECT_OVERVIEW.md` section 2. **[web]** |
| Existing PCVR mod to retarget? | None known for the flat game. |
| Modding community / documentation? | **Large.** BLCMM / OpenBLCMM, the Unofficial Community Patch, `bl-sdk`, Gibbed's tools. The UE3 object model is well documented by the SDK. |
| Native head-tracking support? | No. TrackIR / FreeTrack / SimConnect are not applicable - `opentrackWXR` (Path D) is not a route here. |

### The port

| Question | Answer |
|---|---|
| **Real VR port or injection port?** | **Real VR port** - XrApi 0.5, 6DOF, in-process. Injection (InputAPI) is not viable because head-look needs mouse *motion*, which InputAPI cannot send **[docs]**, and because we have genuine code access through the renderer and the UE3 object model. |
| Target headsets | Quest 3 primary (owner's device). Quest 2 / Pico as best-effort. `TBD` - confirm. |
| Presentation mode | **Mono VR (`MODE_VR=1`, `MODE_3D=0`) is the shipping target.** `MODE_VR=0` while in menus. AER (`MODE_3D=2`) only if measured as affordable. SBS not planned. |
| Ship a non-VR mode as a fallback? | Yes, player-selectable, as both a comfort and a performance option. |
| Recommended container resolution | `TBD` - must be measured. Start at **1400x1400** (square suits the panel, and `MaintainYFOV` narrows horizontally rather than letterboxing **[verified]**). Fall back to 1280x1280 or lower if the frame rate demands it. Do not start at 2800x1400; supersampling is a luxury this game may not afford. |
| First-person framing | First-person. |
| Hands in view? | The **weapon** follows the dominant hand. No new hand art - BL2's first-person mesh is arms-plus-gun already. |
| If injection: mouse motion needed? | N/A - not an injection port. |
| Multiplayer in scope? | `TBD`. The architecture deliberately keeps the game's own firing and networking code so co-op is *not deliberately broken*, but "tested and supported" is a bigger commitment. Ask the owner. |

### Constraints

| Question | Answer |
|---|---|
| May the result be published? Where? | `TBD` - presumably GitHub under MIT, matching `Interstellar-Marines-WXR`. Confirm. |
| May any of the game's source appear in the repository? | **There is no source to leak**, and no cooked package, asset or config file from the install may be committed either. `.gitignore` already excludes build output. |
| Release-owned files | `TBD` - none yet; the repo has no `Release/` directory. |
| Who verifies on hardware? | The owner, on a Quest 3 through WinlatorXR. Loop is slow - one change per deploy. |
| Prior work to inherit or discard? | `Docs/MetaAI/BL2_MonoVR_WinlatorXR/` - a scaffold. **Discard as a codebase, keep as a design sketch.** Reasons in `BUG_FIXES.md`. |

---

## 2. Hard rules

Carried from `Docs/general_wxr/CLAUDE.md`. Binding on every pass, including
agents and scripts.

- **The sync pixel is not optional.** Echo `HMD_SYNC` exactly as received, red
  channel, **green 0**, 10x10, top-left, drawn last. Never rescale or
  gamma-correct it - `XrFramesync` calibrates itself and the remapping is gated
  on `g == 0` **[docs]**.
- **The window must be borderless and fill the display**, re-applied every
  frame.
- **Normalise every quaternion on ingest**, unconditionally.
- **Swap handedness once, at ingest.** Never test the handedness flag again for
  anything spatial.
- **Convert each pose independently of the head.** Share only the recentre
  origin. Hands that swing with head angle is the most common bug in this
  family of ports.
- **Freeze on the last good pose** when packets drop. Never zero.
- **Measure offsets in the frame you will use them in.** If a tuned value only
  works where it was found, the frame is wrong - stop tuning and fix the frame.
- **Instrument before theorising.** Add the log line, read the number.
- **One verbose flag, off by default**, for all diagnostics including file
  dumps.
- **Embed `__DATE__ __TIME__`**, log it on line 1, and make the deploy script
  refuse a binary older than the newest source file.
- **One change per deploy.** Nothing is done until the owner has verified it on
  hardware.
- **Nothing from the game install is committed** - no cooked packages, no
  config files, no assets.
- **Performance is measured on-device**, with WinlatorXR's counters. No frame
  timing in the mod.

---

## 3. What exists today

### Repository layout, as planned

```
  src/                  the mod's own source, our licence
    d3d9/                 the four-export proxy, device wrapper, sync pixel
    xrapi/                XrApi 0.5 client: handshake, UDP in/out, pose maths
    game/                 UE3 hooks: camera, weapon, input, HUD
    third_party/
      unrealsdk/          git submodule, LGPL-3.0, linked as shared library
      minhook/            git submodule, if unrealsdk's hooks do not suffice
  build/                BL2VR.vcxproj + build-and-deploy.ps1
  tools/                packaging
  Docs/
    general_wxr/          the framework (do not edit per-game findings in here)
    MetaAI/               first planning pass, superseded
    ClaudeCode/           these documents
  Release/              shippable artifacts
```

### Current state

Nothing is built. The repository contains a licence, a one-line README, a
`.gitignore`, and `Docs/`. There is no `src/`, no project file, no build
script, and no deploy script. `Docs/MetaAI/BL2_MonoVR_WinlatorXR/` holds about
1,100 lines of scaffold from an earlier planning pass; it has **never been
compiled**, its game-side hooks are empty stubs with `TODO` comments, and
`BUG_FIXES.md` lists eleven defects in it - including one that would stop the
game loading at all. Treat it as a design sketch that establishes the shape,
not as a starting codebase.

### The blocker

**Nobody knows whether flat Borderlands 2 runs acceptably in a WinlatorXR
container.** It is a 2012 CPU-bound UE3 title on an x86 emulator on a mobile
chip, and there is no published benchmark. Every milestone after stage 1 is
contingent on that answer, and the cheapest way to get it is for the owner to
install the unmodified game in a container and look at the frame counter.

Nothing should be written until that is known.

---

## 4. Approach

### Injection vector

**A `d3d9.dll` proxy next to `Borderlands2.exe`, with `unrealsdk` linked into
it for the UE3 object model.**

One binary, in-process, 32-bit. Why:

- **`d3d9.dll` is a static import of the executable** **[verified]**, so the
  proxy is loaded by the Windows loader before the game's first instruction -
  earlier and more reliably than any hook we could install.
- **Only four exports need forwarding** **[verified]**: `Direct3DCreate9`,
  `D3DPERF_BeginEvent`, `D3DPERF_EndEvent`, `D3DPERF_SetOptions`. Missing any
  of the three `D3DPERF_` stubs is a hard load failure, not a degradation.
- **The renderer is where three of the four non-negotiables live** - the sync
  pixel, the borderless window, and the per-frame tick that drains the socket.
  Being the D3D9 device wrapper gets all of them for free.
- **`unrealsdk` already solves the hard half of the game side.** It is a
  maintained C++20 library that locates `GObjects`, `GNames` and the hook
  machinery for Borderlands 2 and exposes UE3 objects and function hooks
  **[web]**. Writing our own sigscans for `ProcessEvent` - which is what
  `Docs/MetaAI/` proposed, against a 2014-era signature - would be
  reimplementing a maintained library badly.

**Routes considered and rejected:**

| Route | Why not |
|---|---|
| A separate native plugin under the existing `willow2-mod-manager` `ddraw.dll` | Forces every player to install the mod manager, and gives us no clean path to the D3D9 device. Still worth keeping as a *development* path - see "prototype in Python" below. |
| Our own `ProcessEvent` sigscan, no `unrealsdk` | Reinventing a maintained library, and version-fragile. |
| A 64-bit helper process with shared memory | Mixed bitness forces IPC; `Docs/general_wxr/TIPS.md` records that exact design being abandoned in the Shogo port and the project becoming dramatically simpler. |
| `OXRWXR` + `OpenComposite` over Borderlands 2 VR | Three extra translation layers, single-player, no DLC, no mods. |
| InputAPI gesture injection (Path B / C) | InputAPI cannot move the mouse, so head-look is impossible through it **[docs]**. |

**Prototype the game side in Python first.** Before writing a line of C++
against `unrealsdk`, install `willow2-mod-manager` on the desktop and use its
Python console to find out, live and in minutes, what the C++ would otherwise
take days to discover: which object holds the camera, what the property names
and types are, which function to hook, and what the engine's unit scale
actually is. Then implement the answer in C++. This is "instrument before
theorising" applied to the game's object model.

**One caution, and it is load-bearing:** `unrealsdk` supports **one injection
per process**, so two components both linking it statically will conflict
**[web]**. If a player has `willow2-mod-manager` installed as well as our DLL,
that is two. Build `unrealsdk` with `UNREALSDK_SHARED` and link the shared
library, which is the documented remedy, and test the combination early -
"our mod and the Python SDK coexist" is a feature a Borderlands 2 audience will
expect.

### Decisions taken deliberately

| Decision | Choice | Why |
|---|---|---|
| Bitness | **32-bit** | the game is; no OpenXR runtime to load; stays in-process |
| Renderer | **keep the game's D3D9** | known-good under DXVK, which is the tuned path; stereo needs no modern API |
| Wrappers | **none shipped** | another layer inside three existing ones; WinlatorXR can inject ReShade itself if a player wants to |
| Process model | **in-process, one DLL** | no IPC, no second binary to start and reap |
| XrApi version | **0.5** | human-readable in a log, the UDP Tester speaks it, poses arrive without declaring spaces, and 0.5 adds the InputAPI listener at no cost **[docs]** |
| Presentation | **Mono VR (`1`,`0`)**; `MODE_VR=0` in menus; AER only if measured | a 2012 UE3 title on a mobile chip; mono is one render per frame |
| UE3 object access | **`unrealsdk` as a submodule** | maintained, BL2-supported, saves writing sigscans |
| Game-side discovery | **`willow2-mod-manager` Python console on the desktop** | minutes instead of days, and it is a measurement not a guess |
| Supersampling | **container resolution, no code** | the runtime filters on downsample - but see the performance position; this game may not afford it |
| Settings surface | **in-game menu**, with a dev-only flag file | editing an INI on a headset is painful **[docs]** |
| Unit scale | **measured, not assumed** | `Docs/MetaAI/` asserts 50 UU per metre; the UE3 nominal is 100. Neither is to be trusted - see Feature 2 |

---

## 5. Milestones

From `Docs/general_wxr/FIRST_STEPS.md`, specialised for this game. Live status
is tracked in `CURRENT_STATUS.md`; this is the definition of each.

- [ ] **0 - Feasibility and intake.** Section 1 of this document complete. The
      two SDK repositories read. The decision to mod the flat game confirmed
      with the owner.
- [ ] **1 - Unmodified game runs in a container.** Launch path established
      (does it need Steam in the container?). Intro movies, mouse capture, save
      paths and the launcher dealt with. `bAutoDetectSettings=false`, post
      processing off, **and verified still off after a relaunch**. Frame rate
      recorded at two or three container resolutions, and on Box64 versus FEX.
      **This is the go/no-go gate.**
- [ ] **2 - Our code loads.** `d3d9.dll` with four exports forwarded. One log
      file, first line the `__DATE__ __TIME__` stamp. `build-and-deploy.ps1`
      with the freshness guard, written *now*, before there is anything hard to
      debug. Confirm the game still launches with the proxy in place, on device
      as well as desktop - DXVK's `d3d9.dll` is the one being forwarded to, and
      that is a difference from the desktop.
- [ ] **3 - We own the window.** Borderless, filling the container display,
      re-applied every frame, idempotent. Watch for UE3 rebuilding the window
      on mode changes.
- [ ] **4 - XrApi 0.5 client.** Handshake files written *after* the runtime has
      wiped the directory; `system` read for the headset name; socket on 7872
      drained every frame keeping the newest; 28 floats + sync + bools parsed
      with trailing tokens tolerated; every quaternion normalised; outbound
      packet on 7278 every frame. Verified against the UDP Tester on the
      desktop. Then **one raw packet captured on the actual headset** and
      diffed field by field, and the button table built from what *moved* in a
      device log - not from the table in `XRAPI_V0_4_DESC.md`.
- [ ] **5 - Sync pixel.** `HMD_SYNC` echoed exactly, red channel, green 0,
      10x10, top-left, drawn **last** and **into the back buffer**. See
      Feature 1 - this is the step most likely to be silently wrong in a UE3
      game.
- [ ] **6 - 6DOF head camera, mono.** The big one. Orientation first, then
      position. Recentre bound immediately. Freeze on the last good pose.
      Checkpoint: does the world stay still when the owner holds their head
      still, does it swim when they turn quickly, does leaning move the view
      the right way *and the right amount*, is the horizon level.
- [ ] **7 - Presentation mode confirmed and measured.** Mono accepted as the
      shipping mode, or AER measured and chosen. `MODE_VR=0` on menus.
- [ ] **8 - Weapon on the dominant hand.** Pose converted independently of the
      head. Aim vector from the weapon. Firing still done by the game.
- [ ] **9 - Full input mapping, owner-confirmed before implementation.**
- [ ] **10 - Comfort and polish.** HUD scale for square resolutions, recentre
      behaviour, head bob and weapon sway off, haptics as decaying durations,
      in-game settings menu.

---

## 6. Features

### Feature 1 - The sync pixel in a UE3 deferred renderer

**Goal.** WinlatorXR can tell which head pose each presented image belongs to,
so head tracking feels locked rather than swimmy. Nothing else in the project
matters more (`Docs/general_wxr/XRAPI_V0_4_DESC.md` section 5).

**Can it be done at all?** Yes, but **not the obvious way**, and this is the
first thing to get right. UE3 does not render to the back buffer; it renders
the scene to its own render targets, post-processes, and resolves. Three
distinct failure modes follow, and the sibling Interstellar Marines port has
already hit the third:

1. Drawing in `Present` with no scene active. `DrawPrimitiveUP` outside a
   `BeginScene`/`EndScene` pair fails with `D3DERR_INVALIDCALL` and draws
   nothing. The inherited scaffold does exactly this.
2. Drawing into whatever render target happens to be bound - which in a UE3
   frame is usually **not** the back buffer. The audit of the Interstellar
   Marines port names this: "the sync pixel is very likely being drawn into an
   offscreen render target".
3. Drawing before the Scaleform UI, which then covers it. BL2's HUD and menus
   are Scaleform/GFx and draw late.

**What must be proven first.** That a quad drawn from our `Present` wrapper
reaches the presented image. The cheapest experiment: draw a **large, bright,
obvious** square - not the near-black sync value - and look at the screen. If
you cannot see a 200x200 magenta block, the sync pixel is not landing either,
and no amount of reasoning about colour values will help. Prove visibility
before you care about correctness.

**Approach.**

1. In the `Present` wrapper, before forwarding: `GetBackBuffer(0,0,MONO,...)`,
   `SetRenderTarget(0, backbuffer)`, `SetDepthStencilSurface(nullptr)`, set the
   viewport to the full buffer.
2. Capture state with a state block **created once** and `Capture()`d per frame
   - not `CreateStateBlock(D3DSBT_ALL)` every frame, which allocates.
3. Point sampling on all three filters; alpha blending, fog, Z, scissor and
   stencil off; no pixel or vertex shader; `D3DFVF_XYZRHW | D3DFVF_DIFFUSE`.
4. `BeginScene`, draw two triangles covering `(0,0)-(10,10)`, `EndScene`.
5. Colour is `D3DCOLOR_XRGB(sync, 0, aerEyeFlag)` with `sync` **echoed exactly
   as received** and green **0**.
6. `Apply()` the state block, release nothing per frame, forward `Present`.

**Fallbacks.** Ranked: (a) hook the device's last `EndScene` of the frame
instead of `Present`; (b) `ColorFill` the back-buffer rect directly, which
needs no scene and no state at all and is worth trying *first* as a diagnostic
even if the final code draws a quad; (c) UE3's `Canvas` through a `PostRender`
hook, which is what the inherited plan tried and which `Docs/MetaAI/` reports
being culled in cinematics.

**Settings.** None player-facing. A dev flag for "draw the square big and
bright" and a second for the green mode-indicator square HWXR uses.

**Edge cases and risks.** Device reset on alt-tab or mode change invalidates
the state block and the cached back buffer - re-acquire both after `Reset`.
DXVK may differ from native D3D9 in what it tolerates outside a scene; test on
device, not only on the desktop.

**Acceptance criteria.**
- [ ] A deliberately oversized bright square is visible in the headset.
- [ ] Reduced to 10x10 at the real sync value, head tracking feels **locked**,
      per the owner on a Quest 3.
- [ ] WinlatorXR's own frame-rate readout tracks the game's frame rate - the
      runtime derives FPS by counting changes in this pixel **[docs]**, so a
      plausible number is independent evidence the pixel is being read.

---

### Feature 2 - The engine unit scale

**Goal.** A head movement of 10 cm moves the in-game camera by the right
amount, and so does leaning, crouching and standing.

**Can it be done at all?** Trivially, *once the constant is known*. The risk is
not the code, it is the number.

**What must be proven first.** `Docs/MetaAI/` asserts **50 UU per metre**
without saying how it was established. The Unreal Engine nominal figure is
**100 UU per metre** (1 UU = 1 cm). The classic Unreal-era character is 88 UU
tall and represents roughly a 1.8 m human, which gives about 49 - which is very
likely where 50 came from, and it is a *character-proportion* figure rather
than a world-scale one. The two differ by a factor of two, and getting it wrong
means either a camera glued still or a camera at the far edge of the level.

**Approach.** Measure it, in the frame it will be used in. Via the Python
console on the desktop, read the player pawn's `CollisionHeight`,
`CollisionRadius` and `BaseEyeHeight`, and the camera's actual eye Z above the
pawn's feet. Compare against a nominal standing eye height of ~1.65 m. Record
the number, the properties it came from, and the date, in section 9 of this
document. Put it in **one named constant**, `kUnitsPerMetre`, and never write a
bare multiplier anywhere else.

**Fallbacks.** If the pawn's proportions turn out to be stylised enough to be
useless as a ruler (Borderlands is a cartoon, and this is a real possibility),
fall back to a known-geometry measurement: a doorway, a vehicle, or a weapon
model whose real-world size is unambiguous. Last resort, tune by eye on device
- and if the tuned value then fails to transfer between levels, the frame is
wrong, not the number.

**Acceptance criteria.**
- [ ] The constant is derived from a named, logged measurement, not tuned.
- [ ] A 50 cm sidestep in the room moves the view by what looks like 50 cm, in
      two different levels, with no per-level trim.

---

### Feature 3 - 6DOF head camera

**Goal.** The head drives the game camera. Looking, leaning, crouching.

**Can it be done at all?** This is the one real unknown in the project and the
reason to prototype in Python first. BL2 is a UE3 game whose view comes from a
chain - `WillowPlayerController` rotation, `WillowPlayerPawn` eye height, a
`PlayerCamera` with camera modifiers, plus camera animations for recoil and
cutscenes. Several points in that chain could carry our pose, and they are not
equally good:

| Candidate | Comment |
|---|---|
| `PlayerController.ProcessViewRotation` | the engine's intended place to modify view rotation; gives orientation, not position |
| `WillowPlayerController.PlayerTick` POST | what `Docs/MetaAI/` proposed; runs every tick but *before* the camera update, so the camera may overwrite us |
| `Camera.UpdateViewTarget` POST | the last word on the final view point and rotation - the strongest candidate for both position and orientation |
| `PlayerController.CalcCamera` | used when there is no camera actor; probably not the live path |
| A custom `CameraModifier` | the engine's own extension point, and worth checking for because it composes with the game's recoil and cutscene animations instead of fighting them |

**What must be proven first.** Which of those actually determines the rendered
view in BL2, established by hooking each in Python and watching what a written
value does on screen. One afternoon in the console settles what would otherwise
be a week of C++ and deploys.

**Approach.** Orientation first, in the mono presentation, with nothing else
changed.

1. Recentre: keep the inverse of the pose captured at recentre time; apply it
   to orientation and to position-minus-origin **[docs]**.
2. Rotate the canonical OpenXR axes by the quaternion, *then* flip for
   handedness. Do not mirror the quaternion itself.
3. UE3 is left-handed, **Z up, X forward, Y right**; XrApi is right-handed,
   **+Y up, -Z forward**, metres **[docs]**. The mapping is therefore
   approximately `ue = (-xr.z, xr.x, xr.y) * kUnitsPerMetre` - **[unverified]**,
   and the sign of each axis is to be established by moving the headset one
   axis at a time and reading the log, not by argument.
4. UE3 `FRotator` components are **int16-scaled: 65536 = 360 degrees**. A
   degrees-to-rotator slip is a 182x error and looks like total chaos rather
   than a subtle bug - expect to meet it once.
5. Then position. Then a recentre binding, immediately; it will be used
   constantly.
6. Freeze on the last good pose when packets drop. Never zero.
7. Disable head bob, weapon sway and camera shake - they fight a head-driven
   camera and are a comfort problem in a headset.

**Risks.** Yaw double-applied, because the game is also applying mouse look.
Cutscenes and vehicle cameras owning the view. Death and respawn resetting it.
`MODE_VR=0` on menus means the pose stream *stops*, so expect a gap on the way
back in and hold the last pose across it **[docs]**.

**Acceptance criteria.**
- [ ] Owner, on device: the world stays still when the head is still.
- [ ] No swim or lag on a fast turn.
- [ ] Leaning moves the view the right way and the right amount.
- [ ] The horizon is level.
- [ ] Entering and leaving a menu, a cutscene and a vehicle does not leave the
      camera stuck or snapped.

---

### Feature 4 - Weapon on the dominant hand, and free aim

**Goal.** The gun sits in the dominant hand and points where that hand points;
the shot goes where the gun points.

**Can it be done at all?** Yes, and there is a worked precedent in this
family: the sibling Interstellar Marines port places the weapon model at the
hand while **leaving the muzzle and the firing logic on the body**, and its
README calls that "anti-cheat safe". The same split is the right one here for a
different reason: it keeps BL2's own `WillowWeapon` firing path, and with it
co-op and the game's damage model.

**What must be proven first.** Whether the aim vector can be written without
replacing the firing code - the candidates being `WillowWeapon`'s muzzle
location and the controller's adjusted aim. Check in Python what BL2 actually
calls when a shot is fired, and hook the narrowest thing that works.

**Approach.** Model first, aim second, and separately:

1. Convert the dominant hand's pose **independently of the head**, sharing only
   the recentre origin. A hand pose routed through the head transform is the
   most common bug in this family **[docs]**.
2. Place the first-person mesh at the hand. Measure the grip offset **in the
   gun's frame**, not the hand's - a grip is a fixed property of the model,
   whereas the hand's axes rotate with the wrist and values found at one wrist
   angle mean nothing at another **[docs]**.
3. Only then override the aim vector.
4. Handedness is swapped **once, at ingest**. After that, no spatial code tests
   the flag again.

**Risks.** BL2 has hundreds of weapon models with differing geometry, and the
grip offset is per-model in the worst case. Start with a single offset and see
how badly it fails before building anything per-weapon. Weapon-replacing mods
exist and will break any art-dependent assumption - degrade to stock behaviour
rather than rendering something broken.

**Acceptance criteria.**
- [ ] Owner, on device, with **controller overlays visible in the screenshot**
      - this is the datum that makes placement a measurement instead of a
      guess **[docs]**.
- [ ] Correct in both handedness modes.
- [ ] Shots land where the gun points, in single-player and in co-op.

---

### Feature 5 - Input mapping

**Goal.** Every game action reachable from the controllers.

**What must be proven first.** The **button table**, built from a device log of
the raw bool string with the owner pressing each control in turn - not from the
table in `XRAPI_V0_4_DESC.md`, which says to do exactly this **[docs]**.

**Approach.** Write the table out as *VR action -> game action*, one line each,
have the owner confirm it **before** implementing, then implement. Decide
deliberately which actions are **held** versus **edge-triggered**: a VR trigger
is a held state, and a game waiting for a key-down edge will wait forever.
Starting point, to be confirmed, not assumed:

| VR input | Game action |
|---|---|
| Left stick | move |
| Right stick X | turn (smooth or snap - a setting) |
| Right trigger | fire |
| Right grip | reload |
| Left trigger | aim down sights |
| Left grip | melee, or crouch |
| A / B | use, jump |
| X / Y | grenade, action skill |
| Left stick click | sprint |
| Right stick click | crouch |
| Left menu | pause - and the point at which `MODE_VR` goes to 0 |

**Risks.** BL2 is a loot game with a lot of UI - inventory, skill trees, the
map, vending machines - all Scaleform, all mouse-driven. A flat menu read
through a VR projection is unusable, which is why `MODE_VR=0` on menus is the
plan: it stops the pose stream, restores the runtime's own mouse control and
drops the frame out of immersive presentation **[docs]**. Getting that
transition right is a feature in its own right, not a detail.

---

### Feature 6 - HUD and comfort

**Goal.** A readable HUD at square and high container resolutions, and the
comfort settings a standalone player needs.

**Approach.** In roughly this order of player impact: HUD scale derived from
measured width against a reference width, with a *strength* control rather than
a raw value **[docs]**; recentre and height; post-processing confirmed off and
*staying* off; haptics as decaying durations - sending 0 does not clear them
**[docs]**; and an in-game settings menu rather than an INI, because a tester
on a headset cannot easily edit files.

---

## 7. Verification

**Build and deploy.** `build/build-and-deploy.ps1`. It must wipe intermediates,
build Release|Win32, read the `__DATE__ __TIME__` stamp out of the binary and
**refuse to deploy** if it is older than the newest source file, then copy and
verify with a SHA-256 comparison printing `match=True` **[docs]**. Write it at
stage 2. The freshness guard is the single highest-value line in it.

**Confirm the right binary is running.** First line of the log carries the
build stamp. Check it before interpreting any device report.

**Desktop pass.** XrApi UDP Tester against the desktop build. This proves *the
code path runs*. It does **not** prove the values are right: the tester's
quaternions are often not unit length and its hand separation does not match a
real headset **[docs]**.

**On-device pass** - the only real gate:

- [ ] per-feature acceptance criteria above
- [ ] head tracking still feels locked
- [ ] frame rate acceptable at the recommended container size, read from
      WinlatorXR's own counter
- [ ] correct in both right- and left-handed mode
- [ ] the game still launches with no headset present (desktop fallback path)
- [ ] previously working features unaffected - name the ones the change could
      plausibly have touched, not all of them

---

## 8. Risks

| Risk | Likelihood | What we do about it |
|---|---|---|
| **The game is simply too slow in a container to be worth playing** | **High** | Stage 1 is a go/no-go gate before any code. Mono only. Low container resolution. FEX versus Box64 measured. If it fails, say so and stop. |
| The sync pixel never reaches the presented frame, because UE3 renders to offscreen targets | High | Feature 1: prove visibility with an oversized bright square before caring about correctness. Three ranked fallbacks. |
| The camera is owned by a chain we cannot cleanly write | Medium | Prototype in Python before writing C++. Non-VR mode (`MODE_VR=2`) remains a shippable product with no camera work at all. |
| `unrealsdk`'s one-injection-per-process limit collides with a player's `willow2-mod-manager` | Medium | Build it `UNREALSDK_SHARED` and test the combination at stage 2, not at release. |
| BL2 needs Steam running inside the container | Medium | Owner establishes the launch path at stage 1. WinlatorXR's store mode is the expected answer. |
| `bAutoDetectSettings` silently restores graphics settings | **Confirmed present** **[verified]** | Set false, then verify after a relaunch - and say so when handing over a build. |
| A saved config on the test device overrides a shipped default | Medium | Say so explicitly with every build handed over. |
| A hook behaves differently under Box64/FEX and Wine than on the desktop | Medium | Prove the hook on device at stage 2, before any feature depends on it. |
| Tuned values do not transfer from tester to device | High | Measure in the frame of use. A value that only works where it was found means the frame is wrong. |
| UE3 rotator units (65536 = 360 degrees) misread as degrees | High, once | Expect it; it is obvious when it happens. |
| Weapon-replacing community mods break art-dependent offsets | Medium | Degrade to stock behaviour rather than rendering something broken. |
| Co-op breaks because the camera detach changed something networked | Medium | Keep the game's firing and networking paths. Test host and client once there is anything to test. |

---

## 9. What we learned

> Append as you go. Not code structure - git shows that. The things that are
> invisible afterwards.

### Borderlands 2 needs a very small `d3d9.dll` proxy

Established 31 Oct 2026 by parsing the import table of
`Binaries/Win32/Borderlands2.exe`. The executable imports exactly four
functions from `d3d9.dll`: `Direct3DCreate9`, `D3DPERF_BeginEvent`,
`D3DPERF_EndEvent`, `D3DPERF_SetOptions`. All four must be exported by the
proxy or the loader fails the process at start - a missing `D3DPERF_` stub is
not a degradation, it is a game that will not launch. The inherited scaffold
exports only `Direct3DCreate9`.

### `AspectRatio_MaintainYFOV` makes square container resolutions safe to try

`WillowGame/Config/DefaultEngine.ini:1372`. A square mode such as 1400x1400
keeps the vertical FOV and narrows the horizontal one, rather than letterboxing
or stretching. Square-ish resolutions suit a headset panel, so this is a cheap
thing to try early - unlike the usual fight to get a game of this era to accept
an unusual mode.

### Nothing is known about this game's performance in this stack

Searched 31 Oct 2026; no benchmark of Borderlands 2 under Winlator or
WinlatorXR on a Quest 3 appears to exist. What is known is that the game is
CPU-bound on desktop hardware, which is the worst shape for an x86 translation
layer. Do not quote a frame-rate figure to anyone until the owner has measured
one. `Docs/MetaAI/PROJECT_PLAN.md` carries numbers from a Call of Duty 4 port
and an estimate that FEX is "15-30% faster than Box64 for UE3"; both are
labelled as estimates there and neither is evidence about this game.
