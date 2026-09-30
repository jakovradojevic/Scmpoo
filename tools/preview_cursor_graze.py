"""Render docs/images/cursor_graze_preview.gif — offline replay of the cursor graze (states 166/167).

Mirrors the game logic: one tick per 108 ms timer, the prop spin advances every tick,
the sheep frame table advances every third tick.
"""
from __future__ import annotations

import struct
import sys
from pathlib import Path

from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parent))
from generate_alien_sheep_bmp import load_pixels  # noqa: E402

ROOT = Path(__file__).resolve().parents[1]
CELL = 40
TICK_MS = 108
SCALE = 3
BITE_1 = 1001
BITE_GONE = 1004
FRAMES = [
    58, 1001, 60, 61, 60, 61, 60, 61, 58, 1002, 60, 61, 60, 61, 60, 61, 2, 58, 1003,
    60, 61, 60, 61, 60, 61, 58, 1004, 60, 61, 60, 61, 60, 61, 3, 0,
]


def load_sheet(resource: int) -> Image.Image:
    data = (ROOT / "Scmpoo" / f"{resource}.bmp").read_bytes()
    img, pal_off, w, h, n = load_pixels(data)
    top = img[::-1]
    pal = [struct.unpack_from("<BBBB", data, pal_off + i * 4) for i in range(n)]
    key = top[0][0]
    out = Image.new("RGBA", (w, h))
    for y in range(h):
        for x in range(w):
            i = top[y][x]
            b, g, r, _ = pal[i]
            out.putpixel((x, y), (r, g, b, 0 if i == key else 255))
    return out


SHEETS: dict[int, Image.Image] = {}


def sprite(index: int, mirrored: bool) -> Image.Image:
    sheet_idx, cell = divmod(index, 16)
    res = 101 + sheet_idx
    if res not in SHEETS:
        SHEETS[res] = load_sheet(res)
    im = SHEETS[res].crop((cell * CELL, 0, cell * CELL + CELL, CELL))
    if mirrored and not 9 <= index <= 14:
        im = im.transpose(Image.FLIP_LEFT_RIGHT)
    return im


def main() -> None:
    # facingDirection = 1: sheep faces left, prop sits at spriteX - 40 (unmirrored).
    sprite_x = 60
    prop_x = sprite_x - CELL
    sheep_index = 3
    stage = 0
    spin = 0
    prop_alive = True
    counter = 0
    frame_pos = 0
    frames: list[Image.Image] = []

    def snapshot() -> None:
        canvas = Image.new("RGBA", (140, CELL + 8), (0, 128, 128, 255))
        if prop_alive:
            canvas.alpha_composite(sprite(352 + stage * 8 + spin, False), (prop_x, 4))
        canvas.alpha_composite(sprite(sheep_index, False), (sprite_x, 4))
        frames.append(canvas.convert("RGB").resize((canvas.width * SCALE, canvas.height * SCALE), Image.NEAREST))

    snapshot()
    while True:
        if prop_alive:
            spin = (spin + 1) % 8
        if counter < 2:
            counter += 1
            snapshot()
            continue
        counter = 0
        idx = FRAMES[frame_pos]
        frame_pos += 1
        if idx == 2:
            sprite_x -= 8
            sheep_index = 2
            snapshot()
            continue
        if BITE_1 <= idx <= BITE_GONE:
            if idx == BITE_GONE:
                prop_alive = False
            else:
                stage = max(stage, idx - BITE_1 + 1)
            idx = FRAMES[frame_pos]
            frame_pos += 1
        if idx == 0:
            snapshot()
            break
        sheep_index = idx
        snapshot()

    for _ in range(8):
        frames.append(frames[-1])
    out = ROOT / "docs" / "images" / "cursor_graze_preview.gif"
    frames[0].save(out, save_all=True, append_images=frames[1:], duration=TICK_MS, loop=0)
    print(f"Wrote {out} ({len(frames)} frames, {len(frames) * TICK_MS / 1000:.1f}s)")

    strip = Image.new("RGB", (frames[0].width, frames[0].height * 6))
    for i, t in enumerate(range(0, len(frames), max(1, len(frames) // 6))):
        if i >= 6:
            break
        strip.paste(frames[t], (0, i * frames[0].height))
    strip.save(ROOT / "docs" / "images" / "cursor_graze_preview_strip.png")


if __name__ == "__main__":
    main()
