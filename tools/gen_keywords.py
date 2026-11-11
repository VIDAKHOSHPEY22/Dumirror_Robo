#!/usr/bin/env python3
"""
gen_keywords.py — Auto-generate keywords.txt from faces.def

Reads src/faces.def and regenerates keywords.txt so Arduino IDE
highlights every face name and id automatically.

Ignores:
  - Lines starting with //   (single-line comments)
  - Everything inside /* */  (block comments)
  - Empty lines

Usage:
    python tools/gen_keywords.py
"""

import re
from pathlib import Path

# ---- locate project root (this file is in tools/) ----
ROOT = Path(__file__).resolve().parent.parent
FACES_DEF = ROOT / "src" / "faces.def"
OUTPUT = ROOT / "keywords.txt"

if not FACES_DEF.exists():
    raise SystemExit(f"❌ Could not find {FACES_DEF}")

raw = FACES_DEF.read_text()

# ---- strip /* ... */ block comments ----
cleaned = re.sub(r'/\*.*?\*/', '', raw, flags=re.DOTALL)

# ---- strip // line comments ----
cleaned = re.sub(r'//.*$', '', cleaned, flags=re.MULTILINE)

# ---- parse X(FACE_XXX, FACE_XXX_ID, "name") lines ----
pattern = re.compile(r'X\(\s*(\w+)\s*,\s*(\w+)\s*,\s*"([^"]+)"\s*\)')
entries = pattern.findall(cleaned)

# ---- safety: reject placeholder names ----
entries = [e for e in entries if e[1] != 'FACE_XXX_ID' and e[2] != 'xxx']

# ---- parse X_ANIM(TAG, ID, "name", ANIM, frames, loop) lines ----
anim_pattern = re.compile(r'X_ANIM\(\s*(\w+)\s*,\s*(\w+)\s*,\s*"([^"]+)"\s*,[^)]*\)')
anim_entries = [(a, b, c) for a, b, c in anim_pattern.findall(cleaned)
                if b != 'FACE_XXX_ID' and c != 'xxx']
entries = entries + anim_entries

if not entries:
    raise SystemExit("❌ No X(...) lines found in faces.def")

print(f"✅ Found {len(entries)} faces: {', '.join(e[2] for e in entries)}")

# ---- build the keywords.txt content ----
lines = []
lines.append("#######################################\n")
lines.append("# Datatypes (KEYWORD1)\n")
lines.append("#######################################\n")
lines.append("DumirrorRobo\tKEYWORD1\n")
lines.append("DumirrorFace\tKEYWORD1\n")
lines.append("Frame\tKEYWORD1\n")
lines.append("Animation\tKEYWORD1\n")
lines.append("DumirrorError\tKEYWORD1\n")
lines.append("\n")

lines.append("#######################################\n")
lines.append("# Methods & Functions (KEYWORD2)\n")
lines.append("#######################################\n")
for m in ("begin", "showFace", "showFaceByName", "faceIndex", "update",
          "getBitmap", "getName", "getNameP", "faceCount", "currentFace",
          "playAnimation", "stopAnimation", "updateAnimation",
          "isAnimationPlaying", "lastError", "showText", "showCenteredText",
          "clearText", "setTextSize", "setTextColor",
          "setTextPosition", "display", "oled"):
    lines.append(f"{m}\tKEYWORD2\n")
lines.append("\n")

lines.append("#######################################\n")
lines.append("# Constants (LITERAL1)\n")
lines.append("#######################################\n")
for _bitmap, enum_id, _name in entries:
    lines.append(f"{enum_id}\tLITERAL1\n")
lines.append("FACE_STATIC_COUNT\tLITERAL1\n")
lines.append("FACE_COUNT\tLITERAL1\n")
for c in ("DUMIRROR_OK", "DUMIRROR_ERR_DISPLAY_NOT_FOUND", "DUMIRROR_ERR_CONFIG_MISMATCH",
          "DUMIRROR_ERR_INVALID_FACE", "DUMIRROR_ERR_INVALID_ANIMATION", "DUMIRROR_ERR_NOT_STARTED",
          "DUMIRROR_PIN_DEFAULT", "DUMIRROR_PROFILE_FULL", "DUMIRROR_PROFILE_COMPACT",
          "DUMIRROR_PROFILE_MINIMAL"):
    lines.append(f"{c}\tLITERAL1\n")
lines.append("\n")

lines.append("#######################################\n")
lines.append("# String literals (LITERAL1)\n")
lines.append("#######################################\n")
for _bitmap, _enum_id, name in entries:
    lines.append(f'"{name}"\tLITERAL1\n')

OUTPUT.write_text("".join(lines))
print(f"✅ Wrote {OUTPUT} ({len(entries)} faces)")


#  python tools\gen_keywords.py