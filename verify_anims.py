#!/usr/bin/env python3
"""
verify_anims.py - validate all anim_*.h files against:
  1. Structural correctness (1024 bytes, static_assert, duration, loop)
  2. faces.def consistency (FRAMES and LOOP match the .h file)
  3. Optional: compare against PNGs in images/ (if present)

Auto-detects the layout:
    src/faces/faces.def + src/faces/anim_*.h
    src/faces.def       + src/anim_*.h
    src/faces.def       + src/faces/anim_*.h

Handles:
    - const Frame FACE_X_ANIM[] PROGMEM = { ... };  (PROGMEM optional)
    - const Frame FACE_X_ANIM[] = { ... };
    - const uint8_t FACE_X_N[] PROGMEM = { ... };
    - const uint8_t FACE_X_N[] = { ... };
    - #define FACE_X_N FACE_ANYTHING  (alias — no bitmap expected)
    - FACE_NORMAL referenced directly (comes from face_normal.h)
    - debug comments like "// row 13, byte 3: 0x3F -> 0x00"

Usage:
    python verify_anims.py                  # structural + faces.def
    python verify_anims.py --pngs           # also check PNGs
    python verify_anims.py --strict         # fail on warnings too
    python verify_anims.py --anim sneeze    # only one animation
    python verify_anims.py --faces-dir src  # force a specific directory
"""
import argparse
import re
import sys
from pathlib import Path

GREEN  = "\033[92m"
RED    = "\033[91m"
YELLOW = "\033[93m"
CYAN   = "\033[96m"
DIM    = "\033[2m"
RESET  = "\033[0m"
BOLD   = "\033[1m"

BITMAP_BYTES = 128 * 64 // 8   # 1024


# ─────────────────────────────────────────────────────────────
#  Auto-detect paths
# ─────────────────────────────────────────────────────────────
def find_faces_def(start: Path):
    candidates = [
        start / "faces.def",
        start / "src" / "faces.def",
        start / "src" / "faces" / "faces.def",
    ]
    for c in candidates:
        if c.exists():
            return c
    for p in start.rglob("faces.def"):
        return p
    return None


def find_anim_headers(faces_def_path: Path):
    base = faces_def_path.parent
    if any(base.glob("anim_*.h")):
        return base
    if (base / "faces").exists() and any((base / "faces").glob("anim_*.h")):
        return base / "faces"
    for p in base.rglob("anim_*.h"):
        return p.parent
    return base


# ─────────────────────────────────────────────────────────────
#  Parsing helpers
# ─────────────────────────────────────────────────────────────
def strip_comments(text: str) -> str:
    """Remove /* */ block comments and // line comments."""
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    text = re.sub(r"//[^\n]*", "", text)
    return text


def find_aliases(text: str) -> set:
    """Find '#define FACE_XXX_N FACE_ANY' aliases (any FACE_* target)."""
    aliases = set()
    for m in re.finditer(r"#define\s+(\w+)\s+(\w+)\s*$", text, re.M):
        target = m.group(2)
        if target.startswith("FACE_"):
            aliases.add(m.group(1))
    return aliases


def _extract_braced_array(text: str, name: str):
    """
    Extract the body between the outermost { and }; for a given
    'const uint8_t NAME[] ... = { ... };' declaration.

    Uses a brace counter to handle nested { } correctly, so any '};'
    inside a static_assert or comment won't confuse it.

    Returns the body string, or None if the array isn't found.
    """
    # Match declaration up to the first '{' after '='
    decl_re = re.compile(
        rf"const\s+uint8_t\s+{re.escape(name)}\s*\[\s*\]\s*(?:PROGMEM\s*)?=\s*\{{",
        re.S
    )
    m = decl_re.search(text)
    if not m:
        return None

    start = m.end()   # first char after '{'
    depth = 1
    i = start
    while i < len(text):
        ch = text[i]
        if ch == '{':
            depth += 1
        elif ch == '}':
            depth -= 1
            if depth == 0:
                return text[start:i]
        i += 1
    return None   # unterminated brace


def parse_header(path):
    """Parse an anim_*.h file. Returns dict with frames, loop, frames_count."""
    text = Path(path).read_text(encoding="utf-8")

    # ── 1. Extract FACE_XXX_ANIM[] array ──
    m = re.search(
        r"const\s+Frame\s+(\w+)\s*\[\s*\]\s*(?:PROGMEM\s*)?=\s*\{(.*?)\}\s*;",
        text, re.S
    )
    if not m:
        return None
    anim_name = m.group(1)
    anim_body = m.group(2)

    anim_body_clean = strip_comments(anim_body)
    entries = re.findall(r"\{\s*(\w+)\s*,\s*(\d+)\s*\}", anim_body_clean)
    frames = [(name, int(dur)) for name, dur in entries]

    # ── 2. FACE_XXX_FRAMES ──
    fm = re.search(r"constexpr\s+uint8_t\s+(\w+_FRAMES)\s*=\s*([^;]+);", text)
    frames_decl = fm.group(2).strip() if fm else None

    # ── 3. FACE_XXX_LOOP ──
    lm = re.search(r"constexpr\s+bool\s+(\w+_LOOP)\s*=\s*(true|false)\s*;", text)
    loop = (lm.group(2) == "true") if lm else None

    # ── 4. All aliases ──
    aliases = find_aliases(text)

    # ── 5. Extract every bitmap using a BRACE COUNTER ──
    text_no_comments = strip_comments(text)
    bitmaps = {}
    for bm in re.finditer(
        r"const\s+uint8_t\s+(\w+)\s*\[\s*\]\s*(?:PROGMEM\s*)?=\s*\{",
        text_no_comments
    ):
        name = bm.group(1)
        body = _extract_braced_array(text_no_comments, name)
        if body is None:
            continue
        data = [int(x, 16) for x in re.findall(r"0x([0-9A-Fa-f]{2})", body)]
        bitmaps[name] = data

    return {
        "path": path,
        "anim_name": anim_name,
        "frames": frames,
        "frames_count": len(frames),
        "frames_decl": frames_decl,
        "loop": loop,
        "bitmaps": bitmaps,
        "aliases": aliases,
    }


def parse_faces_def(path):
    """Return list of X_ANIM entries."""
    text = Path(path).read_text(encoding="utf-8")
    entries = []
    for m in re.finditer(
        r"X_ANIM\s*\(\s*(\w+)\s*,\s*(\w+)\s*,\s*\"([^\"]+)\"\s*,\s*(\w+)\s*,\s*(\d+)\s*,\s*(true|false)\s*\)",
        text
    ):
        entries.append({
            "tag": m.group(1),
            "id": m.group(2),
            "name": m.group(3),
            "arr": m.group(4),
            "frames": int(m.group(5)),
            "loop": m.group(6) == "true",
        })
    return entries


# ─────────────────────────────────────────────────────────────
#  PNG loader (optional)
# ─────────────────────────────────────────────────────────────
def png_to_bytes(path, threshold=128):
    try:
        from PIL import Image
    except ImportError:
        return None
    im = Image.open(path).convert("L")
    if im.size != (128, 64):
        im = im.resize((128, 64), Image.NEAREST)
    data = bytearray(BITMAP_BYTES)
    for y in range(64):
        for x in range(128):
            if im.getpixel((x, y)) >= threshold:
                data[y * 16 + x // 8] |= (0x80 >> (x % 8))
    return list(data)


# ─────────────────────────────────────────────────────────────
#  Report
# ─────────────────────────────────────────────────────────────
class Report:
    def __init__(self):
        self.errors = 0
        self.warnings = 0
        self.oks = 0

    def ok(self, msg):
        self.oks += 1
        print(f"  {GREEN}OK{RESET}   {msg}")

    def warn(self, msg):
        self.warnings += 1
        print(f"  {YELLOW}WARN{RESET} {msg}")

    def err(self, msg):
        self.errors += 1
        print(f"  {RED}ERR{RESET}  {msg}")


# ─────────────────────────────────────────────────────────────
#  Checks
# ─────────────────────────────────────────────────────────────
def check_structure(info, report):
    name = info["anim_name"]
    frames = info["frames"]
    bitmaps = info["bitmaps"]
    aliases = info["aliases"]

    if info["frames_decl"]:
        if "sizeof" in info["frames_decl"]:
            report.ok(f"{name}: FRAMES is auto-computed (sizeof)")
        else:
            try:
                declared = int(info["frames_decl"])
                if declared == info["frames_count"]:
                    report.ok(f"{name}: FRAMES={declared} matches {info['frames_count']} entries")
                else:
                    report.err(f"{name}: FRAMES={declared} != {info['frames_count']} entries")
            except ValueError:
                report.warn(f"{name}: FRAMES is not a plain integer: {info['frames_decl']}")
    else:
        report.err(f"{name}: FACE_*_FRAMES not found")

    if info["loop"] is not None:
        report.ok(f"{name}: LOOP={str(info['loop']).lower()}")
    else:
        report.err(f"{name}: FACE_*_LOOP not found")

    for bname, data in bitmaps.items():
        if len(data) == BITMAP_BYTES:
            report.ok(f"{name}: {bname} = {len(data)} bytes")
        else:
            report.err(f"{name}: {bname} = {len(data)} bytes (expected {BITMAP_BYTES})")

    referenced = {f[0] for f in frames}
    for ref in referenced:
        if ref == "FACE_NORMAL":
            continue
        if ref in aliases:
            continue
        if ref not in bitmaps:
            report.err(f"{name}: frame references '{ref}' but no bitmap found")

    total = sum(d for _, d in frames)
    report.ok(f"{name}: total duration = {total} ms across {len(frames)} frames")
    for fname, dur in frames:
        if dur == 0:
            report.err(f"{name}: frame '{fname}' has duration 0")
        elif dur > 5000:
            report.warn(f"{name}: frame '{fname}' duration {dur} ms (>5s, is this intentional?)")


def check_faces_def(info, faces_entries, report):
    name = info["anim_name"]
    arr = info["anim_name"]

    matches = [e for e in faces_entries if e["arr"] == arr]
    if not matches:
        report.err(f"{name}: no X_ANIM entry in faces.def for '{arr}'")
        return
    if len(matches) > 1:
        report.err(f"{name}: {len(matches)} X_ANIM entries in faces.def for '{arr}' (expected 1)")
        return

    entry = matches[0]

    if entry["frames"] == info["frames_count"]:
        report.ok(f"faces.def: FRAMES={entry['frames']} matches .h")
    else:
        report.err(f"faces.def: FRAMES={entry['frames']} != .h ({info['frames_count']})")

    if entry["loop"] == info["loop"]:
        report.ok(f"faces.def: LOOP={str(entry['loop']).lower()} matches .h")
    else:
        report.err(f"faces.def: LOOP={str(entry['loop']).lower()} != .h ({str(info['loop']).lower()})")


def check_pngs(info, images_dir, report, threshold=128):
    name = info["anim_name"]
    short = name.replace("FACE_", "").replace("_ANIM", "").lower()

    if not images_dir.exists():
        report.warn(f"{name}: images/ directory not found, skipping PNG check")
        return

    frames = info["frames"]
    bitmaps = info["bitmaps"]
    aliases = info["aliases"]

    for idx, (fname, _) in enumerate(frames, start=1):
        if fname == "FACE_NORMAL" or fname in aliases:
            continue

        candidates = [
            images_dir / f"oled-128x64-{short}-{idx}.png",
            images_dir / f"{short}-{idx}.png",
            images_dir / f"{short}_{idx}.png",
            images_dir / f"{short}.png",
        ]
        png = next((p for p in candidates if p.exists()), None)
        if not png:
            report.warn(f"{name}: no PNG for frame {idx} ('{fname}')")
            continue

        png_data = png_to_bytes(png, threshold)
        if png_data is None:
            report.warn(f"{name}: PIL not installed, cannot read {png.name}")
            continue

        hdr_data = bitmaps.get(fname)
        if hdr_data is None:
            report.err(f"{name}: frame {idx} bitmap '{fname}' not in header")
            continue

        if png_data == hdr_data:
            report.ok(f"{name}: frame {idx} matches {png.name}")
        else:
            diff = sum(1 for a, b in zip(png_data, hdr_data) if a != b)
            first = next((i for i, (a, b) in enumerate(zip(png_data, hdr_data)) if a != b), -1)
            report.err(
                f"{name}: frame {idx} differs from {png.name} "
                f"({diff} bytes, first at offset {first})"
            )


# ─────────────────────────────────────────────────────────────
#  Main
# ─────────────────────────────────────────────────────────────
def main():
    ap = argparse.ArgumentParser(description="Verify all anim_*.h files")
    ap.add_argument("--faces-dir", default=None)
    ap.add_argument("--images-dir", default="images")
    ap.add_argument("--anim")
    ap.add_argument("--pngs", action="store_true")
    ap.add_argument("--threshold", type=int, default=128)
    ap.add_argument("--strict", action="store_true")
    a = ap.parse_args()

    if a.faces_dir:
        def_path = Path(a.faces_dir) / "faces.def"
        if not def_path.exists():
            sys.exit(f"error: {def_path} not found")
    else:
        def_path = find_faces_def(Path.cwd())
        if def_path is None:
            sys.exit("error: could not find faces.def")

    anim_dir = find_anim_headers(def_path)
    images_dir = Path(a.images_dir)

    print(f"{BOLD}{CYAN}=== Verifying Dumirror animations ==={RESET}")
    print(f"  faces.def : {def_path}")
    print(f"  anim dir  : {anim_dir}")
    print(f"  images dir: {images_dir}")
    print(f"  check PNGs: {a.pngs}")
    print()

    faces_entries = parse_faces_def(def_path)
    print(f"  {len(faces_entries)} X_ANIM entries in faces.def")
    print()

    headers = sorted(anim_dir.glob("anim_*.h"))
    if a.anim:
        headers = [h for h in headers if a.anim.lower() in h.stem.lower()]

    if not headers:
        sys.exit(f"error: no anim_*.h files found in {anim_dir}")

    total_report = Report()

    for header_path in headers:
        print(f"{BOLD}{header_path.name}{RESET}")
        info = parse_header(header_path)
        if info is None:
            total_report.err(f"{header_path.name}: could not parse (no FACE_*_ANIM[] found)")
            print()
            continue

        check_structure(info, total_report)
        check_faces_def(info, faces_entries, total_report)

        if a.pngs:
            check_pngs(info, images_dir, total_report, a.threshold)

        print()

    print(f"{BOLD}=== Summary ==={RESET}")
    print(f"  {GREEN}OK:{RESET}   {total_report.oks}")
    print(f"  {YELLOW}WARN:{RESET} {total_report.warnings}")
    print(f"  {RED}ERR:{RESET}  {total_report.errors}")
    print()

    if total_report.errors > 0:
        print(f"{RED}{BOLD}FAIL{RESET} - {total_report.errors} error(s)")
        return 1
    if total_report.warnings > 0 and a.strict:
        print(f"{YELLOW}{BOLD}FAIL (strict){RESET} - {total_report.warnings} warning(s)")
        return 1
    print(f"{GREEN}{BOLD}PASS{RESET} - all checks passed")
    return 0


if __name__ == "__main__":
    sys.exit(main())