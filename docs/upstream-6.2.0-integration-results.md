# Moonlight-Qt 6.2.0 Integration Results

**Completed:** 2026-10-30  
**Integration worktree:** `D:\projects\repos\moonlight-qt-v620-integration`  
**Original checkout:** `D:\projects\repos\moonlight-qt` (left on its original branch and commit)

## Final Revisions

| Item | Branch/ref | Commit |
|---|---|---|
| Parent integration | `integration/upstream-6.2.0` | `b1378ca8ec3aa855360b3fa77a7789d4fa0e5140` |
| Original custom parent | `agent/apollo-vibepollo-features` | `9723d44304d6e72a6b2f49afec9770844c4b0500` |
| Upstream parent | `v6.2.0` | `de2467e433821664cdd2224aad8c89a625be1ad9` |
| Common-c integration | `integration/v6.2.0-server-command` | `053d299d80f677d4d05021f196f8b59f507e4ba6` |
| Common-c upstream base | upstream | `f900dd4767759c7b9d0e93bcea666b55c69ea62f` |
| qmdnsengine | upstream gitlink | `920c097ffa742e2968290f15d4dde6693aec02e5` |
| SDL GameControllerDB | unchanged gitlink | `8d9fefd7b810f2541f78cc7a8ccbd185bc84c7a5` |

The parent integration commit is a true two-parent merge of the custom fork and the peeled upstream v6.2.0 tag. `git merge-base --is-ancestor refs/tags/v6.2.0^{} HEAD` passed.

The report itself is a documentation follow-up to the source integration commit above. No branch or commit was pushed.

## Worktree Isolation

The integration was performed in a new worktree created with:

```powershell
git worktree add D:\projects\repos\moonlight-qt-v620-integration `
    -b integration/upstream-6.2.0 `
    9723d44304d6e72a6b2f49afec9770844c4b0500
```

The original checkout remained at:

```text
branch: agent/apollo-vibepollo-features
HEAD:   9723d44304d6e72a6b2f49afec9770844c4b0500
```

Its existing `build/` directory and untracked `AGENTS.md`/`docs/` files were not changed, reset, removed, or used for the integration build.

## Common-C Update

The common library was initialized at the custom `a0f555f` gitlink, then a separate local branch was created directly from upstream `f900dd4`. The original server-command commit was cherry-picked onto that base:

```powershell
git switch -c integration/v6.2.0-server-command `
    f900dd4767759c7b9d0e93bcea666b55c69ea62f
git cherry-pick a0f555fec748b7c6cf9cc8353b3b50f76f797cee
```

Result: `053d299d80f677d4d05021f196f8b59f507e4ba6`.

The delta from `f900dd4` remains exactly 22 inserted lines in three files:

- `src/ControlStream.c` — 17 lines;
- `src/Limelight-internal.h` — 1 line;
- `src/Limelight.h` — 4 lines.

`LiSendExecServerCmd()`, packet type `0x3000`, control channel `0x08`, encrypted Gen 7 enforcement, and reliable delivery are preserved. The branch also inherits upstream packet-loss recovery, extended-key support, Steam controller identification, MbedTLS PSA modernization, and license changes.

The common checkout's `origin` points to `https://github.com/Mythandar/moonlight-common-c.git`; upstream is configured separately as `upstream`. The commit is local only due to the no-push restriction. Until it is eventually pushed, another clone cannot resolve the parent gitlink from GitHub.

## Merge Conflicts and Resolutions

Command used:

```powershell
git merge --no-commit 'refs/tags/v6.2.0^{}'
```

### Literal conflict

Only one literal conflict occurred: `moonlight-common-c/moonlight-common-c`. Git could not automatically combine the unrelated custom and upstream gitlinks. It was resolved by recording combined commit `053d299d...`, not by selecting either old side.

`qmdnsengine` was advanced to upstream `920c097...`. `SDL_GameControllerDB` remained unchanged. `.gitmodules` retains the Mythandar common-c URL.

### Auto-merged files reviewed semantically

Git auto-merged all parent source overlaps. They were reviewed rather than accepted solely because they lacked conflict markers:

- `app/app.pro`: custom clipboard, permissions, commands, and Quick Menu sources remain registered; upstream library changes are present.
- `app/gui/StreamSegue.qml`: upstream livelock-safe spinner timing coexists with the custom Ctrl+Shift+Q hint.
- `app/main.cpp`: executable-relative portable detection remains; upstream XDG desktop matching is present.
- `app/settings/streamingpreferences.cpp`: clipboard and virtual-display settings remain default-off; upstream 1080p default is present.
- input files: upstream extended-key state, Enter/Numpad Enter distinction, moved key/mouse helpers, Steam controller support, and reduced redundant gamepad sends coexist with custom shortcuts, Quick Menu interception, neutral controller state, and mouse-mode override.
- renderer files: upstream color-range handling coexists with Quick Menu overlay positioning in every custom renderer path.
- `scripts/build-arch.bat`: upstream `vswhere`/`jom` discovery and signing behavior coexist with the custom portable marker staging.

No conflict markers remained. `git diff --check` passed.

## Custom Feature Preservation

Static source validation confirmed:

- `virtualDisplay=1` is still added when enabled;
- no `VirtualDisplayCapable` gate was reintroduced;
- clipboard GET/POST paths and the 1 MB/loop-prevention manager remain present;
- Set Clipboard, Read Clipboard, and Server Command permission-aware behavior remains present;
- `LiSendExecServerCmd()` is declared, linked, and compiled;
- Quick Menu remains available via Ctrl+Shift+`\` without Alt;
- Select+L1+R1+Y still clears local axes/buttons, sends a neutral controller packet, and releases mouse-emulation X1/X2 buttons;
- Desktop Mouse Mode still overrides automatic fullscreen mouse mode;
- the session-scoped manual mouse-mode override remains initialized per stream;
- all Quick Menu overlay branches remain present;
- standard Sunshine behavior remains the fallback when Apollo-only capabilities/permissions are absent.

As an additional comparison, the merged tree's custom delta relative to v6.2.0 remains the previously reviewed 71 files, 1,435 insertions, and 210 deletions. Custom-only backend/session/settings paths were unchanged by the upstream merge.

## Build Results

Dependencies were deployed with:

```powershell
powershell -ExecutionPolicy Bypass -File .\setup-deps.ps1
```

The documented invocation without `-ExecutionPolicy Bypass` was initially blocked by the machine's PowerShell execution policy. The process-scoped bypass succeeded and downloaded both Windows x64 and ARM64 dependency archives.

The full x64 release build used Qt 6.11.1 MSVC 2022:

```powershell
cmd.exe /c "set PATH=D:\projects\Qt\6.11.1\msvc2022_64\bin;%PATH%&& scripts\build-arch.bat release"
```

**Result: successful.** Moonlight, the updated common library, MSI, portable ZIP, and symbol archive compiled/linked/packaged with zero errors.

### Non-fatal warnings

- MSVC C5287 in bundled ENet for bitwise operations involving different enum types.
- MSVC C4005 for SDL/Qt `__SSE__` and `__SSE2__` macro redefinitions.
- LNK4291 for Qt's entry-point object not being built with `/guard:ehcont`.
- Two WiX WIX1076/ICE69 warnings for Start Menu/Desktop shortcut components referencing `MoonlightExe` in another component within the same feature.
- Benign messages for nonexistent clean-output directories on the first isolated build and absent optional deployment files/styles.

No warning was introduced specifically by an Apollo/Vibepollo source file, and no compile or link repair was required.

## Automated Validation

### qmdnsengine tests

The available qmdnsengine CMake/Qt test suite was configured and built with Qt 6.11.1. All seven tests passed:

```text
TestBrowser   passed
TestCache     passed
TestDns       passed
TestHostname  passed
TestProber    passed
TestProvider  passed
TestResolver  passed

100% tests passed, 0 failed
```

### Other automated/static checks

- Parent and common-c `git diff --check`: passed.
- Upstream v6.2.0 ancestry check: passed.
- Conflict-marker scan: passed.
- Common-c delta check: exactly three files/22 insertions.
- Custom source-registration and invariant searches: passed.
- Full Windows x64 compile, link, deployment, MSI validation, and ZIP creation: passed.
- Executable metadata: `FileVersion=6.2.0.0`, `ProductVersion=6.2.0.0`.

There is no comprehensive top-level Moonlight-Qt unit-test suite in this repository.

## Portable Identity Validation

Local development deployment:

```text
build\deploy-x64-release\portable.dat.inactive  present (3 bytes)
build\deploy-x64-release\portable.dat           absent
```

Portable archive listing:

```text
portable.dat           present (3 bytes)
portable.dat.inactive  absent
```

This validates build-time marker placement. Runtime verification that registry identity is reused, and that portable INI storage follows the executable when launched from another working directory, remains in the manual checklist.

## Generated Artifacts

All artifacts are isolated under `D:\projects\repos\moonlight-qt-v620-integration\build`.

| Artifact | Exact path | Bytes | SHA-256 |
|---|---|---:|---|
| Development executable | `build\deploy-x64-release\Moonlight.exe` | 2,772,480 | `25A906EE105FCBF4C46C0EF50576CAB08742C3D33763DF0267400E2A9A5901B2` |
| MSI | `build\build-x64-release\Moonlight.msi` | 26,673,152 | `51EA7597EAA12134DCDE4C68B6AE33DAE3D5E358E3CD956F491A650043D6CC93` |
| Portable ZIP | `build\installer-x64-release\MoonlightPortable-x64-6.2.0.zip` | 34,328,132 | `FC2923026E53F075C13735BBDF7A7DC073255AF09A792631B80C0D7CAC4E7C50` |
| Debug symbols | `build\symbols-x64-release\MoonlightDebuggingSymbols-x64-6.2.0.zip` | 39,835,875 | `88291E54BD894F86C9EDA8AEC50CED80D31BDEC11B610D06757AD9D06B2D0F75` |

## Outstanding Problems and Limitations

No critical merge or build problem remains.

The following are intentionally outstanding:

1. The parent and common-c integration branches are local and unpushed.
2. The MSI was built and validated by WiX but not installed.
3. No real Sunshine/Vibepollo stream was launched during automated validation.
4. Controller input leakage, mouse capture behavior, clipboard permissions, server commands, virtual display creation, and pairing persistence require interactive host/client testing.
5. macOS, AppImage/VAAPI, Steam Link, ARM64, and non-D3D renderers were not built in this Windows x64 pass.
6. The executable was not launched from alternate working directories; portable-path behavior was validated statically and through archive markers only.

## Manual Testing Checklist

- [ ] Standard Sunshine discovery, pairing, app list, launch, resume, quit, stream, and disconnect with Apollo settings disabled.
- [ ] Vibepollo permission display matches the paired client.
- [ ] Clipboard client-to-host with Set Clipboard allowed/denied.
- [ ] Clipboard host-to-client with Read Clipboard allowed/denied.
- [ ] Clipboard loop prevention, focus lifecycle, Unicode plain text, and 1 MB rejection.
- [ ] Virtual display off versus on; confirm `virtualDisplay=1` and actual virtual display creation.
- [ ] Server-command list, execution, success/failure reporting, permission denial, and destructive confirmation.
- [ ] Ctrl+Shift+`\` opens/closes Quick Menu without requiring Alt and without stuck modifiers.
- [ ] Select+L1+R1+Y opens/closes Quick Menu without host-side Y/L1/R1/Select or X1/X2 leakage.
- [ ] Quick Menu actions: disconnect, quit, stats, mouse capture, keyboard capture, fullscreen, clipboard, and commands.
- [ ] Windowed start is mouse-free; fullscreen captures automatically before manual override.
- [ ] Ctrl+Shift+M choice survives later windowed/fullscreen transitions for the current session.
- [ ] Desktop Mouse Mode prevents automatic fullscreen capture and is not circumvented by the new logic.
- [ ] A new stream resets the session-scoped mouse override.
- [ ] Enter and Numpad Enter are distinct; right Ctrl/Alt and navigation keys release correctly.
- [ ] D3D11 rendering on the target GPU, including fullscreen transitions, VRR/frame pacing, overlays, and full/limited-range content.
- [ ] Development deployment reuses existing registry-backed certificate/UUID and does not require pairing again after rebuild.
- [ ] Portable ZIP creates/uses an INI beside the executable, including when launched from another working directory.
- [ ] Packet-loss recovery under induced loss with RFI enabled and disabled.

## Handoff

The integration is complete locally and ready for interactive regression testing. Do not push the parent branch until `053d299d...` is also pushed to the Mythandar common-c fork; otherwise the parent submodule gitlink will be unavailable to other clones.
