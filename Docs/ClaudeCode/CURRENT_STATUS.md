# CURRENT_STATUS

**As of 31 Oct 2026.** Branch `main`, one commit (`f780292`, "Initial commit").

---

## In one line

**Planning only. No code, nothing built, nothing on hardware.** The project is
at milestone 0 and the next action belongs to the owner, not to an agent.

---

## Milestones

From `PROJECT_PLAN.md` section 5.

| | Milestone | State |
|---|---|---|
| 0 | Feasibility assessed, intake complete | **in progress** - the game is characterised, the intake interview is not finished |
| 1 | Unmodified game runs in a container | not started - **this is the blocker** |
| 2 | Our code loads and writes a log with a build stamp | not started |
| 3 | Window borderless and filling the display | not started |
| 4 | XrApi client parses real device packets | not started |
| 5 | Sync pixel drawn, last, into the back buffer | not started |
| 6 | 6DOF head tracking feels locked | not started |
| 7 | Presentation mode chosen and measured | not started |
| 8 | Weapon on the dominant hand | not started |
| 9 | Full input mapping, owner-confirmed | not started |
| 10 | HUD scale, comfort settings, in-game menu | not started |

---

## What exists in the repository

| Path | What it is | Committed? |
|---|---|---|
| `README.md` | one line | yes |
| `LICENSE` | MIT | yes |
| `.gitignore` | C++ build artefacts | modified, uncommitted |
| `BL2-WXR.code-workspace` | VS Code workspace | untracked |
| `Docs/general_wxr/` | the shared WinlatorXR framework, 9 files | untracked |
| `Docs/MetaAI/` | first planning pass + ~1,100-line scaffold | untracked |
| `Docs/ClaudeCode/` | these four documents | new, untracked |

There is **no** `src/`, no `build/`, no project file, no build script, no
deploy script, and no `Release/`.

`git status` at the start of this session showed `.gitignore` modified and
`BL2-WXR.code-workspace` plus `Docs/` untracked, so none of the documentation
is committed yet.

---

## What is verified, and what is only believed

### Verified this session, on this machine

Checked against `E:\Steam\steamapps\common\Borderlands 2`; the full table is in
`PROJECT_OVERVIEW.md` section 3.

- `Borderlands2.exe` is **PE32 / x86**, Direct3D 9, changelist `2863302`.
- It imports exactly **four** functions from `d3d9.dll`, so a proxy needs four
  exports - and the inherited scaffold provides one.
- Input comes in through `DINPUT8.dll` and `XINPUT1_3.dll`.
- All 24 DLC folders are installed.
- `bAutoDetectSettings=true` and `AspectRatioAxisConstraint=MaintainYFOV` in
  `WillowGame/Config/DefaultEngine.ini`.
- The UE3 console key is undefined by default
  (`Engine/Config/BaseInput.ini:229`).

### Believed from documentation, verified elsewhere but not here

Everything in `Docs/general_wxr/` - the XrApi 0.5 wire format, the handshake
files, the sync-pixel behaviour, the four presentation modes, the port
pitfalls. That set is verified against a Quest 3 and against WinlatorXR's own
source, but **not against this game**. Its own advice is to re-verify the
protocol against a live capture at the start of each port; that has not been
done here.

### Believed from the web

- `bl-sdk/unrealsdk` is a C++20 library for UE3 object access and function
  hooks, consumable as a submodule, with Borderlands 2 listed as supported, and
  it supports **one injection per process**.
- `bl-sdk/willow2-mod-manager` is the current Python SDK for BL2, installed as
  `Binaries/Win32/ddraw.dll`, and it expects the executable to be named
  `Borderlands2.exe`.
- Borderlands 2 VR is a separate, single-player-only product.

Neither SDK has been installed or run.

### Not known at all

- Whether the unmodified game launches in a WinlatorXR container.
- Whether it needs Steam running inside the container.
- What frame rate it manages, at any resolution, on Box64 or FEX.
- Which object in the UE3 chain actually determines the rendered view.
- The engine's unit scale (`Docs/MetaAI/` says 50 UU/m; the UE3 nominal is 100;
  neither has been measured here).
- Whether a quad drawn from a `Present` wrapper reaches the presented frame at
  all in a UE3 renderer.

---

## Inherited work: `Docs/MetaAI/BL2_MonoVR_WinlatorXR/`

About 1,100 lines of a Visual Studio project: a `d3d9.dll` proxy with a full
`IDirect3DDevice9` wrapper, an XrApi UDP client, a hand-rolled 5-byte
trampoline hooker, and empty stubs for the game-side hooks.

**Keep it as a design sketch. Do not build on it.** The device-wrapper
boilerplate is genuinely useful as a reference for the ~100 forwarding methods.
Everything else needs rewriting:

- It has **never been compiled**. No binary, no build log.
- The game-side hooks - the actual point of the project - are empty functions
  with `TODO` comments. `Hooks::Init()` writes a log line saying it "would
  hook" something.
- It exports one of the four `d3d9.dll` functions the game needs, so as
  written the game would not load.
- It does socket setup, file I/O, thread creation and a **15-second wait loop**
  inside `DllMain`, under the loader lock.
- It sends `MODE_VR=2` (flat window) while describing it as VR mode.
- It does not normalise quaternions, does not own the window, and draws the
  sync pixel outside a scene.

`BUG_FIXES.md` lists these with the fix for each.

---

## The blocker

**Nobody knows whether flat Borderlands 2 runs acceptably in a WinlatorXR
container**, and every milestone from 2 onward is contingent on the answer.
There is no published benchmark, the game is CPU-bound on desktop hardware, and
it is six to fourteen years newer than anything else ported in this family.

This is not something an agent can resolve. It needs the owner, a container and
a frame counter.

---

## Next three actions

1. **Owner, on device, milestone 1.** Install unmodified Borderlands 2 in a
   WinlatorXR container and report: does it launch, what did it take to make it
   launch, and what frame rate does the container report in a firefight - at
   1280x1280, at 1400x1400, and on Box64 versus FEX. Set
   `bAutoDetectSettings=false` and turn post-processing off first, then relaunch
   and check they stayed off.
2. **Owner, five minutes, intake.** Answer the six open questions in
   `PROJECT_OVERVIEW.md` section 8 and fill the `TBD` rows in
   `PROJECT_PLAN.md` section 1.
3. **Agent, desktop, no deploy needed.** Install `willow2-mod-manager` against
   the desktop install and use its Python console to answer the two questions
   that gate the whole game side: which object determines the rendered view,
   and what the engine's unit scale actually measures as. Both are an afternoon
   in a console and would otherwise be a week of C++ and deploys. Record the
   answers in `PROJECT_PLAN.md` section 9.

Action 3 can proceed in parallel with 1 and 2, and is the only one an agent can
do alone. **Do not start writing the mod before action 1 reports.**
