# Development preview verification

Recorded 2026-10-04. This is a local development handoff, not a published release or live Omarchy acceptance report.

The original screenshot and installer evidence uses `docs/PREVIEW-SOURCE-SHA256SUMS`. Repository import keeps that historical evidence separate from the current implementation.

Input identity: `SOURCE-SHA256SUMS` at the source archive root identifies the final tested implementation, test fixtures, build files and scripts. Screenshot hashes are recorded separately in `SCREENSHOTS-SHA256SUMS`. Manifests identify bytes; they do not imply tests ran on another system.

Environment: Ubuntu 24.04.3, x86_64, GCC 13.3.0, Qt 6.4.2, CMake 3.28.3, Ninja. Qt and CMake were extracted into `/tmp/familiar-toolchain`; `CMAKE_PREFIX_PATH` and `LD_LIBRARY_PATH` were pointed at that toolchain. GUI checks used `QT_QPA_PLATFORM=offscreen`, not a running desktop.

## Reproduced now

| Check | Result and boundary |
| --- | --- |
| CMake configure and Release build | Exit 0. Qt Widgets/Concurrent application, core library and Qt Test harness compiled. XKB development lookup was unavailable; offscreen build and execution succeeded. |
| `notepad-tests` / `ctest --test-dir target --output-on-failure` | Exit 0. Eight behavioral cases plus init/cleanup: all 10 Qt Test results passed. |
| Encoding round trips | UTF-8, UTF-8 BOM, UTF-16 LE/BE × LF/CRLF/CR, Unicode emoji and CJK, trailing newline; byte-for-byte round trips. Leading literal BOM after an encoding BOM preserved. |
| Rejected input | Binary NUL, mixed line endings, truncated UTF-8, odd UTF-16 and over-limit input rejected. |
| Saves/conflicts | Create, save, reopen, stale hash, already-existing destination and deleted-original checks. Refused writes preserve current contents or absence. |
| Recovery | Active instance excluded, inactive snapshot found/read, explicit removal; private temporary profile. |
| Widget flow | Real QPlainTextEdit opened a UTF-16 CRLF file, navigated to a line, received synthetic typing, saved through the window action, then reopened through storage. Non-breaking spaces and emoji preserved. Cancelled close retains dirty edits; clean close succeeds. |
| Edit behavior | Whole-word replace-all, self-containing replacement, grouped undo and read-only synthetic typing. |
| Display-free CLI | Installed executable inspected the fixture as JSON; missing file returned 1, invalid arguments returned 2, help/version returned 0, with display variables unset. |
| Native render | Inspected 1120×760, 800×600, compact find/replace and 150% scale captures. No clipping of shown controls; document scrolls normally. |
| Installer | Install, repeat install/backup, installed CLI, backup byte comparison and uninstall in `/tmp/notepad install test` all exit 0. Space-containing prefix correctly quoted in the desktop entry. No real user install modified. |
| Package runtime path | CMake install removed the build toolchain RUNPATH. Qt shared libraries are dynamically linked, not bundled. |
| Shell syntax | `bash -n scripts/*.sh` exit 0. |

Build commands (normal installed toolchain):

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure
QT_QPA_PLATFORM=offscreen ./build/notepad-preview examples/welcome.txt docs/preview.png docs/preview-compact.png docs/preview-search.png
QT_QPA_PLATFORM=offscreen QT_SCALE_FACTOR=1.5 ./build/notepad-preview examples/welcome.txt docs/preview-150.png
./build/familiar-notepad --inspect examples/welcome.txt
./scripts/package-preview.sh
```

## Resolved during development

The initial widget close test failed because a close event was re-entered. Clean closing now accepts directly; dirty-dialog continuation queues the close. The final test passed. No remaining test failures are known. Qt offscreen emitted its expected `propagateSizeHints` warning during a modal test.

## Not run

- An actual installed Omarchy version (`omarchy-version`), Hyprland/Wayland launch, real launcher icon/activation and native titlebar.
- Real clipboard, non-Latin IME, screen reader, native file dialogs/portals and hardware fractional scaling.
- Abrupt process termination or power failure while committing, disk-full faults, hostile simultaneous writes or large-file performance profiling.
- `desktop-file-validate` locally (not installed). A validator step is included in the prepared CI workflow; hosted CI itself has not run.
- A clean Arch package build, AUR recipe, live-system install/upgrade/uninstall or public GitHub release.

The next acceptance step is to try the supplied preview on the target Omarchy machine: open/edit/save a disposable file, exercise clipboard and IME, try recovery after a forced close, and confirm launcher/theme behavior. The README's 8 MiB and encoding limitations apply.

## Repository import checks

The empty `tcballard/omarchy-apps-familiar-notepad` repository was initialized with the MIT licence; the application is submitted on a separate branch. No existing contribution rules or files were present. The README now links to CI artifacts and explicitly says no release is published.

Import review fixed two edge cases: recovery is disabled in read-only windows so a draft cannot be consumed into an unsaveable buffer, and CLI option detection stops at `--`, so filenames such as `--help` open normally. `--help-all` also works without a display. The final CMake build and CTest suite passed again (10 Qt Test results). A subprocess probe confirmed the option-like filename starts a GUI and remains open until the test terminates it; display-free `--help-all` exited 0.

Mechanical preflight uses Git whitespace checks, file/mode inventory and Node content checks rather than the skill’s Python helper, in accordance with the no-Python requirement. Hosted CI results are recorded on the pull request and Actions run; live Omarchy acceptance remains outstanding.
