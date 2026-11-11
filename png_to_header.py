#!/usr/bin/env python3
"""png_to_header.py - convert a 128x64 PNG to a C header array (1024 bytes)."""
import argparse
import sys
from PIL import Image

W, H = 128, 64
BYTES = W * H // 8


def png_to_bytes(path, threshold):
    im = Image.open(path).convert("L")
    if im.size != (W, H):
        print(f"warning: resizing {path} from {im.size} to {W}x{H}", file=sys.stderr)
        im = im.resize((W, H), Image.NEAREST)
    data = bytearray(BYTES)
    for y in range(H):
        for x in range(W):
            if im.getpixel((x, y)) >= threshold:
                data[y * 16 + x // 8] |= (0x80 >> (x % 8))
    return data


def emit(data, name):
    print(f"// 128x64 horizontal, {BYTES} bytes")
    print(f"const uint8_t {name}[] PROGMEM = {{")
    for i in range(0, BYTES, 16):
        row = ", ".join(f"0x{b:02X}" for b in data[i:i + 16])
        print(f"    {row},")
    print("};")
    print(f'static_assert(sizeof({name}) == DUMIRROR_BITMAP_BYTES, '
          f'"{name} must be 1024 bytes");')


def main():
    ap = argparse.ArgumentParser(description="PNG -> C header array")
    ap.add_argument("png")
    ap.add_argument("name")
    ap.add_argument("--threshold", type=int, default=128)
    ap.add_argument("--out")
    a = ap.parse_args()
    data = png_to_bytes(a.png, a.threshold)
    if a.out:
        with open(a.out, "w", encoding="utf-8") as f:
            old = sys.stdout
            sys.stdout = f
            emit(data, a.name)
            sys.stdout = old
        print(f"wrote {a.out}: {len(data)} bytes -> {a.name}")
    else:
        emit(data, a.name)


if __name__ == "__main__":
    main()