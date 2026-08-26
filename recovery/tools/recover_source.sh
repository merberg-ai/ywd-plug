#!/usr/bin/env bash
set -Eeuo pipefail

ROOT="${1:-$(pwd)}"
UPSTREAM_REPO="https://github.com/infamy/NeonPlug.git"
UPSTREAM_COMMIT="$(tr -d '[:space:]' < "$ROOT/recovery/UPSTREAM_COMMIT")"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

cd "$ROOT"
if [[ -f .ywd-source-recovered && -d src ]]; then
  echo "[INFO] Source is already recovered; refusing to overwrite it."
  exit 0
fi

echo "[INFO] Cloning pinned NeonPlug baseline $UPSTREAM_COMMIT"
git clone --quiet --no-tags "$UPSTREAM_REPO" "$TMP/neonplug"
git -C "$TMP/neonplug" checkout --quiet --detach "$UPSTREAM_COMMIT"
ACTUAL="$(git -C "$TMP/neonplug" rev-parse HEAD)"
[[ "$ACTUAL" == "$UPSTREAM_COMMIT" ]] || { echo "[FAIL] upstream SHA mismatch" >&2; exit 2; }

# Fingerprint the radio engine before any YWD presentation/namespace patch.
UPSTREAM_DM32_HASH="$({ find "$TMP/neonplug/src/radios/dm32uv" -type f -print0 | sort -z | xargs -0 sha256sum; } | sha256sum | awk '{print $1}')"

# Copy the complete implementation/test/build tree, but preserve YWD repository
# governance/docs and avoid bulky upstream promotional artwork.
rsync -a "$TMP/neonplug/" "$ROOT/" \
  --exclude='.git/' \
  --exclude='.github/' \
  --exclude='.gitignore' \
  --exclude='README.md' \
  --exclude='CONTRIBUTING.md' \
  --exclude='LICENSE' \
  --exclude='LICENSE.md' \
  --exclude='NOTICE' \
  --exclude='NOTICE.md' \
  --exclude='demo.gif' \
  --exclude='neonplug_banner.jpg'

python3 "$ROOT/recovery/tools/apply_ywd_recovery.py" "$ROOT"

RECOVERED_DM32_HASH="$({ find "$ROOT/src/radios/dm32uv" -type f -print0 | sort -z | xargs -0 sha256sum; } | sha256sum | awk '{print $1}')"
if [[ "$RECOVERED_DM32_HASH" != "$UPSTREAM_DM32_HASH" ]]; then
  echo "[FAIL] DM-32UV radio engine changed during source recovery" >&2
  echo "upstream : $UPSTREAM_DM32_HASH" >&2
  echo "recovered: $RECOVERED_DM32_HASH" >&2
  exit 3
fi
echo "[OK] DM-32UV radio engine is byte-identical to pinned upstream"

npm ci
npm test -- --run
npm run build
npm run build:single

echo "[OK] Source recovery tests and builds passed"
