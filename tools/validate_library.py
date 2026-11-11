#!/usr/bin/env python3
"""validate_library.py - static consistency checks for the Dumirror_Robo repo.

Checks (exit code 1 on any failure):
  * faces.def entries <-> face headers <-> Dumirror_Config.h switches
  * every bitmap array holds exactly 1024 bytes; names / ids are unique
  * keywords.txt is up to date with faces.def (tools/gen_keywords.py)
  * every include in src/ and examples/ resolves; no stray backups/.vscode
  * every example documents its hardware and avoids duplicate framebuffers
  * version strings agree across library.properties / header / CHANGELOG / README
  * wiring documentation agrees with the TouchPet source for every board
  * optional: --original ORIGINAL.zip proves artwork bytes are unchanged

Usage:  python3 tools/validate_library.py [--original path/to/original.zip]
"""
import argparse, hashlib, re, sys, zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
errors = []
def fail(msg): errors.append(msg); print("  FAIL", msg)
def ok(msg): print("  ok  ", msg)

def strip_comments(t):
    t = re.sub(r'/\*.*?\*/', '', t, flags=re.S)
    return re.sub(r'//.*', '', t)

def bitmaps_in(path):
    """name -> list of byte values for every uint8_t array in a header."""
    t = strip_comments(path.read_text(encoding="utf-8", errors="replace"))
    out = {}
    for m in re.finditer(r'const\s+uint8_t\s+(\w+)\s*\[\s*\d*\s*\]\s*(?:PROGMEM)?\s*=\s*\{(.*?)\};', t, re.S):
        out[m.group(1)] = [int(v, 0) for v in re.findall(r'0[xX][0-9a-fA-F]+|\b\d+\b', m.group(2))]
    return out

def main():
    ap = argparse.ArgumentParser(); ap.add_argument("--original"); args = ap.parse_args()
    src = ROOT / "src"
    defn = strip_comments((src / "faces.def").read_text())
    statics = re.findall(r'^\s*X\(\s*(\w+)\s*,\s*(\w+)\s*,\s*"([^"]+)"\s*\)', defn, re.M)
    anims = re.findall(r'^\s*X_ANIM\(\s*(\w+)\s*,\s*(\w+)\s*,\s*"([^"]+)"\s*,\s*(\w+)\s*,\s*(\d+)\s*,\s*(\w+)\s*\)', defn, re.M)

    print("[faces.def]")
    names = [s[2] for s in statics] + [a[2] for a in anims]
    ids = [s[1] for s in statics] + [a[1] for a in anims]
    (ok if len(set(names)) == len(names) else fail)(f"{len(names)} face names unique")
    (ok if len(set(ids)) == len(ids) else fail)(f"{len(ids)} enum ids unique")
    (ok if (len(statics), len(anims)) == (19, 10) else fail)(f"19 static + 10 animated (found {len(statics)}+{len(anims)})")
    cfg = (src / "Dumirror_Config.h").read_text()
    raw = (src / "faces.def").read_text()
    cpp = (src / "Dumirror_Robo.cpp").read_text()
    for n in names:
        N = n.upper()
        if f"DUMIRROR_ENABLE_FACE_{N}" not in cfg: fail(f"Config switch missing for {n}")
        if f"#if DUMIRROR_ENABLE_FACE_{N}\n" not in raw: fail(f"faces.def guard missing for {n}")
    ok("every face has a config switch and a faces.def guard")

    print("[bitmaps]")
    allbm = {}
    for h in sorted((src / "faces").glob("*.h")):
        for k, v in bitmaps_in(h).items():
            allbm[k] = v
            if len(v) != 1024: fail(f"{h.name}:{k} has {len(v)} bytes")
        if h.name.startswith(("face_", "anim_")) and f'#include "faces/{h.name}"' not in cpp:
            fail(f"{h.name} is not included by Dumirror_Robo.cpp")
    ok(f"{len(allbm)} bitmap arrays, all 1024 bytes")
    for bm, _, n in statics:
        if bm not in allbm: fail(f"static face {n}: array {bm} not defined")
    for tag, _, n, arr, frames, loop in anims:
        t = (src / "faces" / f"anim_{n}.h").read_text()
        if arr not in t: fail(f"anim {n}: {arr} not found")
        if not re.search(rf'{arr}\[\]\s*PROGMEM', t): fail(f"anim {n}: frame table is not PROGMEM")
        durs = re.findall(r'\{\s*\w+\s*,\s*(\d+)\s*\}', t.split(f"{arr}[]")[1].split("};")[0])
        if len(durs) != int(frames): fail(f"anim {n}: {len(durs)} frames in header, faces.def says {frames}")
    ok("animation tables are PROGMEM and match faces.def frame counts")

    print("[keywords]")
    kw = (ROOT / "keywords.txt").read_text()
    for n in names + ids + ["DumirrorRobo", "begin", "getNameP", "faceIndex", "lastError", "oled", "currentFace"]:
        if re.search(rf'^"?{re.escape(n)}"?\t', kw, re.M) is None: fail(f"keywords.txt lacks {n}")
    ok("keywords.txt covers all faces and key API names")

    print("[hygiene]")
    bad = [p for p in ROOT.rglob("*") if p.is_file() and ".git" not in p.parts and
           (p.suffix in (".bak", ".orig", ".pyc") or ".vscode" in p.parts or "__pycache__" in p.parts)]
    (fail if bad else ok)(f"no backup/IDE files {[str(b.relative_to(ROOT)) for b in bad]}" if bad else "no backup/IDE/cache files")
    crlf = [p for p in list(src.rglob("*")) + list((ROOT / "examples").rglob("*.ino")) if p.is_file() and b"\r\n" in p.read_bytes()]
    (fail if crlf else ok)("no CRLF line endings in source" if not crlf else f"CRLF in {[c.name for c in crlf]}")
    for p in list(src.rglob("*.h")) + list(src.rglob("*.cpp")) + list(src.rglob("*.def")):
        for inc in re.findall(r'#include\s+"([^"]+)"', p.read_text()):
            if not (p.parent / inc).exists() and not (src / inc).exists():
                fail(f"{p.relative_to(ROOT)} includes missing {inc}")
    ok("all quoted includes in src/ resolve")

    print("[examples]")
    for ino in sorted((ROOT / "examples").glob("*/*.ino")):
        t = ino.read_text()
        if "HARDWARE CONNECTIONS" not in t: fail(f"{ino.parent.name}: no HARDWARE CONNECTIONS section")
        if re.search(r'Adafruit_SSD1306\s+\w+\s*\(', t): fail(f"{ino.parent.name}: creates its own Adafruit_SSD1306 (duplicate framebuffer)")
        if "Serial.printf" in t: fail(f"{ino.parent.name}: Serial.printf is not available on AVR")
        if ino.parent.name != ino.stem: fail(f"{ino}: sketch name must match folder")
    ok("examples: hardware section present, single framebuffer, portable Serial use")

    print("[versions]")
    ver = re.search(r'^version=(.+)$', (ROOT / "library.properties").read_text(), re.M).group(1).strip()
    hv = re.search(r'DUMIRROR_ROBO_VERSION\s+"([^"]+)"', (src / "Dumirror_Robo.h").read_text()).group(1)
    if hv != ver: fail(f"header version {hv} != library.properties {ver}")
    if f"## {ver}" not in (ROOT / "CHANGELOG.md").read_text(): fail(f"CHANGELOG lacks {ver}")
    if f"version-{ver}-" not in (ROOT / "README.md").read_text(): fail(f"README badge lacks {ver}")
    ok(f"version {ver} consistent")

    print("[wiring docs vs TouchPet source]")
    board_pins = {  # canonical, per official core defaults (see docs/WIRING.md for caveats)
        "ESP32 (classic)": ("GPIO 21", "GPIO 22"), "ESP32-S2": ("GPIO 8", "GPIO 9"),
        "ESP32-S3": ("GPIO 8", "GPIO 9"), "ESP32-C3": ("GPIO 8", "GPIO 9"),
        "Arduino Uno R3": ("A4", "A5"), "Arduino Nano Classic": ("A4", "A5"),
        "Arduino Mega 2560": ("D20", "D21"), "Arduino Leonardo": ("D2", "D3"),
        "ESP8266 (NodeMCU)": ("GPIO 4", "GPIO 5"), "RP2040 (Pico)": ("GP4", "GP5"),
    }
    tp = (ROOT / "examples/TouchPet/TouchPet.ino").read_text()
    wd = (ROOT / "docs/WIRING.md").read_text()
    for b, (sda, scl) in board_pins.items():
        short = b.replace("Arduino ", "")
        for label, text in (("TouchPet.ino", tp), ("docs/WIRING.md", wd)):
            rows = [l for l in text.splitlines() if b in l or short in l]
            if not any(sda in l and scl in l for l in rows): fail(f"{label}: {b} row lacks {sda}/{scl}")
    ok("TouchPet header and WIRING.md agree on SDA/SCL for all boards")

    if args.original:
        print("[artwork vs original]")
        z = zipfile.ZipFile(args.original); changed = 0; compared = 0
        for name in z.namelist():
            m = re.match(r'Dumirror_Robo/src/faces/((?:face|anim)_\w+\.h)$', name)
            if not m: continue
            old = bitmaps_in_text(z.read(name).decode("utf-8", "replace"))
            new = bitmaps_in(src / "faces" / m.group(1))
            for k, v in old.items():
                compared += 1
                if new.get(k) != v: changed += 1; fail(f"bitmap {k} differs from original")
        (ok if changed == 0 else fail)(f"{compared} bitmap arrays byte-identical to the original release")

    print("\nRESULT:", "FAIL (%d)" % len(errors) if errors else "PASS")
    return 1 if errors else 0

def bitmaps_in_text(text):
    tmp = Path("/tmp/_dum_tmp.h"); tmp.write_text(text); return bitmaps_in(tmp)

if __name__ == "__main__":
    sys.exit(main())
