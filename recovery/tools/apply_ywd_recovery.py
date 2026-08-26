#!/usr/bin/env python3
"""Apply the minimal YWD-Plug recovery layer to the pinned NeonPlug source tree.

This script intentionally does not touch src/radios/** or the transport/protocol
implementation. The first recovery checkpoint is about restoring maintainable
source and YWD identity while preserving the known-good DM-32UV engine.
"""
from __future__ import annotations

import json
import re
import sys
from pathlib import Path

ROOT = Path(sys.argv[1] if len(sys.argv) > 1 else ".").resolve()
RECOVERY = ROOT / "recovery"


def read(rel: str) -> str:
    return (ROOT / rel).read_text(encoding="utf-8")


def write(rel: str, text: str) -> None:
    path = ROOT / rel
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8")


def replace_required(rel: str, old: str, new: str, *, minimum: int = 1) -> None:
    text = read(rel)
    count = text.count(old)
    if count < minimum:
        raise SystemExit(f"recovery patch anchor missing in {rel}: {old!r}")
    write(rel, text.replace(old, new))
    print(f"[patch] {rel}: {count} replacement(s)")


def replace_optional(rel: str, old: str, new: str) -> None:
    path = ROOT / rel
    if not path.exists():
        return
    text = path.read_text(encoding="utf-8")
    count = text.count(old)
    if count:
        path.write_text(text.replace(old, new), encoding="utf-8")
        print(f"[patch] {rel}: {count} optional replacement(s)")


# Package identity. Keep upstream version for now; YWD releases will acquire
# their own versioning once the recovered baseline is regression-tested.
pkg_path = ROOT / "package.json"
pkg = json.loads(pkg_path.read_text(encoding="utf-8"))
pkg["name"] = "ywd-plug"
pkg["description"] = "Browser-native multi-radio programming and codeplug management"
pkg_path.write_text(json.dumps(pkg, indent=2) + "\n", encoding="utf-8")

lock_path = ROOT / "package-lock.json"
if lock_path.exists():
    lock = json.loads(lock_path.read_text(encoding="utf-8"))
    lock["name"] = "ywd-plug"
    if isinstance(lock.get("packages"), dict) and isinstance(lock["packages"].get(""), dict):
        lock["packages"][""]["name"] = "ywd-plug"
    lock_path.write_text(json.dumps(lock, indent=2) + "\n", encoding="utf-8")

# Page metadata. The app remains suitable for the existing /plug/ web path,
# but the product identity is YWD-Plug.
index = read("index.html")
index = re.sub(
    r"<title>.*?</title>",
    '<meta name="theme-color" content="#05090d" />\n'
    '    <meta name="application-name" content="YWD-Plug" />\n'
    '    <meta name="description" content="YWD-Plug — browser-native radio programming and codeplug management." />\n'
    '    <title>YWD-Plug // Radio Programming</title>',
    index,
    count=1,
)
write("index.html", index)

# Namespaced browser storage. These are the exact keys already used by the
# earlier YWD build and documented in the recovery notes.
replace_required("src/App.tsx", "neonplug_log_level", "ywdplug_log_level")
replace_required("src/App.tsx", "[NeonPlug]", "[YWD-Plug]")
replace_required("src/store/debugStore.ts", "neonplug-debug-mode", "ywdplug-debug-mode")
replace_required("src/services/codeplugSnapshots.ts", "neonplug-codeplug-snapshots", "ywdplug-codeplug-snapshots")

# YWD native codeplug format. The ZIP payload remains codeplug.json and is
# wire-compatible with the recovered app. Legacy .neonplug remains readable.
replace_required("src/services/codeplugExport.ts", "codeplug-export-${timestamp}.neonplug", "codeplug-export-${timestamp}.ywdplug")
replace_optional("src/services/codeplugExport.ts", "(.neonplug)", "(.ywdplug; legacy .neonplug compatible)")
replace_optional("src/services/codeplugExport.ts", "a .neonplug file", "a .ywdplug or legacy .neonplug file")

app = read("src/App.tsx")
app = app.replace(
    "if (fileExtension === 'neonplug') {",
    "if (fileExtension === 'ywdplug' || fileExtension === 'neonplug') {",
)
app = app.replace(
    "// Check if it's a codeplug file (.neonplug = zipped JSON)",
    "// Check if it's a YWD/legacy codeplug file (zipped JSON)",
)
app = app.replace(
    "File must be a codeplug (.neonplug) or CSV file containing",
    "File must be a codeplug (.ywdplug or legacy .neonplug) or CSV file containing",
)
app = app.replace('accept=".csv,.neonplug"', 'accept=".csv,.ywdplug,.neonplug"')
write("src/App.tsx", app)

replace_required("src/components/layout/Toolbar.tsx", 'accept=".neonplug"', 'accept=".ywdplug,.neonplug"')
replace_required("src/components/layout/Toolbar.tsx", "Import codeplug from file (.neonplug)", "Import codeplug from file (.ywdplug; legacy .neonplug supported)")
replace_required("src/components/layout/Toolbar.tsx", "Export codeplug to file (.neonplug)", "Export codeplug to file (.ywdplug)")

# Product branding in the always-visible shell and startup experience.
replace_required("src/components/layout/StatusBar.tsx", "NEONPLUG", "YWD-PLUG")
startup = read("src/components/ui/StartupModal.tsx")
startup = startup.replace("https://infamy.github.io/NeonPlug/", "https://kj6ywd.net/plug/")
startup = startup.replace("save as neonplug.html", "save as ywd-plug.html")
startup = startup.replace(">NEONPLUG<", ">YWD-PLUG<")
startup = startup.replace("Channel programming software", "Radio programming • CPS • protocol tools")
startup = startup.replace("Import from codeplug file (.neonplug)", "Import .ywdplug (legacy .neonplug supported)")
write("src/components/ui/StartupModal.tsx", startup)

# Offline artifact naming.
replace_optional("src/utils/offlineDownload.ts", "neonplug-offline.zip", "ywd-plug-offline.zip")
replace_optional("src/utils/offlineDownload.ts", "neonplug.html", "ywd-plug.html")

# About tab: recover YWD product identity while explicitly restoring NeonPlug
# upstream links/credit after the textual brand pass.
about_path = ROOT / "src/components/about/AboutTab.tsx"
if about_path.exists():
    about = about_path.read_text(encoding="utf-8")
    about = about.replace("NEONPLUG", "YWD-PLUG").replace("NeonPlug", "YWD-Plug")
    about = about.replace("neonplug.html", "ywd-plug.html").replace("neonplug-offline.zip", "ywd-plug-offline.zip")
    about = about.replace("https://github.com/infamy/YWD-Plug", "https://github.com/infamy/NeonPlug")
    about = about.replace("https://infamy.github.io/YWD-Plug/", "https://kj6ywd.net/plug/")
    about = about.replace("YWD-Plug Repository:", "Upstream NeonPlug Repository:")
    about_path.write_text(about, encoding="utf-8")
    print("[patch] src/components/about/AboutTab.tsx: YWD identity + upstream-link preservation")

# A couple of harmless app-identification strings in helpers/diagnostics.
for path in ROOT.glob("src/**/*.ts*"):
    if "radios" in path.parts:
        continue
    text = path.read_text(encoding="utf-8")
    original = text
    text = text.replace("NeonPlug Connection Debug", "YWD-Plug Connection Debug")
    text = text.replace("neonplug-full-export-", "ywd-plug-full-export-")
    text = text.replace("neonplug-debug-", "ywd-plug-debug-")
    text = text.replace("User-Agent: NeonPlug", "User-Agent: YWD-Plug")
    if text != original:
        path.write_text(text, encoding="utf-8")
        print(f"[patch] {path.relative_to(ROOT)}: diagnostic/helper branding")

# Apply the exact YWD-Hotspot-inspired design-token foundation prepared during
# Phase 1 planning. This intentionally leaves editor semantics untouched.
for rel in ("tailwind.config.js", "src/styles/globals.css"):
    source = RECOVERY / "overlay" / rel
    target = ROOT / rel
    if not source.exists():
        raise SystemExit(f"missing recovery overlay: {source}")
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(source.read_text(encoding="utf-8"), encoding="utf-8")
    print(f"[overlay] {rel}")

(ROOT / ".ywd-source-recovered").write_text(
    "upstream=infamy/NeonPlug\n"
    "commit=8ae184770e03a93959f81c262f2ba9dcb93b0400\n"
    "recovery=YWD-Plug source baseline\n",
    encoding="utf-8",
)

print("[OK] YWD-Plug recovery layer applied")
