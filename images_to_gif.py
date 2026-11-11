#!/usr/bin/env python3
"""
images_to_gif.py - turn images (or Dumirror face .h headers) into an animated GIF.

All output GIFs loop forever (loop=0), like Telegram GIFs.
The --once flag is removed on purpose: previews must always loop.
"""
import argparse
import glob
import os
import re
import sys

from PIL import Image

W, H = 128, 64
BITMAP_BYTES = W * H // 8


def parse_color(text):
    text = text.lstrip("#")
    if len(text) != 6:
        raise argparse.ArgumentTypeError("color must be like ffaacc")
    return tuple(int(text[i:i + 2], 16) for i in (0, 2, 4))


def load_header(path, fg, bg):
    data = [int(x, 16) for x in re.findall(r"0x([0-9A-Fa-f]{2})", open(path, encoding="utf-8").read())]
    if len(data) != BITMAP_BYTES:
        print(f"warning: {path} has {len(data)} bytes (expected {BITMAP_BYTES}); padding/cutting",
              file=sys.stderr)
        data = (data + [0] * BITMAP_BYTES)[:BITMAP_BYTES]
    im = Image.new("RGB", (W, H), bg)
    px = im.load()
    for y in range(H):
        for x in range(W):
            if data[y * 16 + x // 8] >> (7 - x % 8) & 1:
                px[x, y] = fg
    return im


def to_mono(im, threshold, fg, bg):
    gray = im.convert("L")
    out = Image.new("RGB", gray.size, bg)
    out.paste(Image.new("RGB", gray.size, fg),
              mask=gray.point(lambda v: 255 if v >= threshold else 0))
    return out


def expand_inputs(items):
    paths = []
    for item in items:
        if os.path.isdir(item):
            found = sorted(glob.glob(os.path.join(item, "*")))
            paths += [p for p in found
                      if p.lower().endswith((".png", ".jpg", ".jpeg",
                                             ".bmp", ".gif", ".webp", ".h"))]
        elif any(ch in item for ch in "*?["):
            paths += sorted(glob.glob(item))
        else:
            paths.append(item)
    return paths


def main():
    ap = argparse.ArgumentParser(description="Images / face headers -> animated GIF")
    ap.add_argument("inputs", nargs="+",
                    help="files, folders or quoted globs, in play order")
    ap.add_argument("-o", "--out", default="out.gif",
                    help="output file (default out.gif)")
    ap.add_argument("-d", "--duration", type=int, nargs="+", default=[300],
                    help="ms per frame: one value for all, or one per frame")
    ap.add_argument("-s", "--scale", type=int, default=1,
                    help="integer upscale, nearest-neighbour")
    ap.add_argument("--size", help="resize every frame first, e.g. 128x64")
    ap.add_argument("--pingpong", action="store_true",
                    help="play forward then backward")
    ap.add_argument("--mono", action="store_true",
                    help="convert images to 1-bit using the fg/bg colors")
    ap.add_argument("--threshold", type=int, default=128,
                    help="mono threshold 0-255 (default 128)")
    ap.add_argument("--oled", action="store_true",
                    help="shortcut: soft cyan pixels on dark navy, implies --mono")
    ap.add_argument("--fg", type=parse_color, default=(255, 255, 255),
                    help="pixel color hex (header/mono mode)")
    ap.add_argument("--bg", type=parse_color, default=(0, 0, 0),
                    help="background color hex (header/mono mode)")
    # --once was removed on purpose — all GIFs loop forever (like Telegram).
    a = ap.parse_args()

    if a.oled:
        a.fg, a.bg, a.mono = (190, 245, 255), (14, 16, 32), True

    paths = expand_inputs(a.inputs)
    if not paths:
        sys.exit("no input files found")

    size = None
    if a.size:
        m = re.fullmatch(r"(\d+)x(\d+)", a.size)
        if not m:
            sys.exit("--size must look like 128x64")
        size = (int(m.group(1)), int(m.group(2)))

    frames = []
    for p in paths:
        if p.lower().endswith(".h"):
            im = load_header(p, a.fg, a.bg)
        else:
            im = Image.open(p).convert("RGBA")
            flat = Image.new("RGBA", im.size, a.bg + (255,))
            im = Image.alpha_composite(flat, im).convert("RGB")
            if size:
                im = im.resize(size, Image.NEAREST)
            if a.mono:
                im = to_mono(im, a.threshold, a.fg, a.bg)
        if size and p.lower().endswith(".h"):
            im = im.resize(size, Image.NEAREST)
        if a.scale > 1:
            im = im.resize((im.width * a.scale, im.height * a.scale), Image.NEAREST)
        frames.append(im)

    if len(a.duration) not in (1, len(frames)):
        sys.exit(f"-d needs 1 value or exactly {len(frames)} values (got {len(a.duration)})")
    durations = a.duration * len(frames) if len(a.duration) == 1 else list(a.duration)

    if a.pingpong and len(frames) > 2:
        frames = frames + frames[-2:0:-1]
        durations = durations + durations[-2:0:-1]

    if len({f.size for f in frames}) != 1:
        sys.exit("frames have different sizes; use --size WxH")

    # ✅ loop=0 => infinite loop (like Telegram GIFs)
    frames[0].save(
        a.out,
        save_all=True,
        append_images=frames[1:],
        duration=durations,
        loop=0,
        disposal=1,
        optimize=False,
    )
    print(f"wrote {a.out}: {len(frames)} frames, {sum(durations)} ms, "
          f"{frames[0].size[0]}x{frames[0].size[1]}, loop=forever")


if __name__ == "__main__":
    main()