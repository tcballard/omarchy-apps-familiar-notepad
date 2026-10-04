#!/usr/bin/env bash
set -euo pipefail
prefix="${FAMILIAR_INSTALL_PREFIX:-$HOME/.local}"
[[ "$prefix" == /* ]] || { echo 'Install prefix must be absolute.' >&2; exit 1; }
rm -f -- "$prefix/bin/familiar-notepad" "$prefix/bin/familiar-notepad.previous" \
  "$prefix/share/applications/io.github.tcballard.FamiliarNotepad.desktop" \
  "$prefix/share/icons/hicolor/scalable/apps/io.github.tcballard.FamiliarNotepad.svg"
if command -v update-desktop-database >/dev/null; then update-desktop-database "$prefix/share/applications"; fi
echo 'Removed Familiar Notepad. Your documents and recovery drafts have not been touched.'
