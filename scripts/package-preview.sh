#!/usr/bin/env bash
set -euo pipefail
root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
build="${1:-$root/build}"
output="${2:-$root/dist}"
name="familiar-notepad-0.0.1-preview-linux-x86_64"
[[ "$(uname -m)" == x86_64 ]] || { echo 'This preview archive targets x86_64.' >&2; exit 1; }
mkdir -p "$output"
stage="$(mktemp -d)"
trap 'rm -rf -- "$stage"' EXIT
cmake --install "$build" --prefix "$stage/$name"
mkdir -p "$stage/$name/packaging"
cp "$root"/packaging/*.desktop "$root"/packaging/*.svg "$stage/$name/packaging/"
cp -R "$root/scripts" "$root/docs" "$root/examples" "$stage/$name/"
cp "$root/README.md" "$root/LICENSE" "$root/CREDITS.md" "$stage/$name/"
(cd "$stage/$name" && find . -type f ! -name SHA256SUMS -print0 | sort -z | xargs -0 sha256sum > SHA256SUMS)
tar -C "$stage" -czf "$output/$name.tar.gz" "$name"
printf 'Created %s/%s.tar.gz\n' "$output" "$name"
