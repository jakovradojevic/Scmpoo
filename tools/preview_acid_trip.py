"""Render docs/images/acid_trip_preview.gif — offline replay of states 166-174.

Mushroom graze, scared twice, then the acid trip (spin, flip + bounce, roll, upside-down
roll, dizzy blinks) with horn/eye palette cycling, one frame per 108 ms game tick.
"""
from __future__ import annotations

import struct
import sys
from pathlib import Path

from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parent))
from generate_alien_sheep_bmp import load_pixels  # noqa: E402
from preview_cursor_graze import FRAMES as GRAZE_FRAMES, BITE_1, BITE_GONE  # noqa: E402

ROOT = Path(__file__).resolve().parents[1]
CELL = 40
TICK_MS = 108
SCALE = 3
AMAZED = [50, 51, 50, 51, 3, 0]
SPIN = [3, 9, 10, 11, 2, 14, 13, 12]
ROLL = [119, 120, 121, 122, 123, 124, 125, 126]
BLINK = [7, 8, 7, 6, 7, 8, 7, 6]
BOUNCE = [0, -6, -10, -6]
ACID_SHEETS = {11, 18, 24}
# (r, g, b) bright/mid/dark horn shades and pupil, same values as acidColours in Scmpoo.c.
ACID = [
    [(255, 0, 0), (170, 0, 0), (96, 0, 0), (255, 0, 0)],
    [(255, 255, 0), (170, 170, 0), (96, 96, 0), (255, 255, 0)],
    [(0, 255, 0), (0, 170, 0), (0, 96, 0), (0, 255, 0)],
    [(0, 0, 255), (0, 0, 170), (0, 0, 96), (0, 0, 255)],
    [(255, 0, 255), (170, 0, 170), (96, 0, 96), (255, 0, 255)],
    [(128, 128, 0), (96, 96, 0), (56, 56, 0), (160, 160, 0)],
    [(128, 0, 128), (96, 0, 96), (56, 0, 56), (160, 0, 160)],
    [(0, 0, 128), (0, 0, 96), (0, 0, 56), (0, 0, 160)],
    [(0, 128, 0), (0, 96, 0), (0, 56, 0), (0, 160, 0)],
]

RAW: dict[int, tuple[list[list[int]], list[tuple[int, int, int]]]] = {}


def raw_sheet(resource: int):
    if resource not in RAW:
        data = (ROOT / "Scmpoo" / f"{resource}.bmp").read_bytes()
        img, pal_off, w, h, n = load_pixels(data)
        pal = [tuple(data[pal_off + i * 4 : pal_off + i * 4 + 3][::-1]) for i in range(n)]
        RAW[resource] = (img[::-1], pal)
    return RAW[resource]


def sprite(index: int, mirrored: bool, colour: int | None) -> Image.Image:
    sheet, cell = divmod(index, 16)
    top, pal = raw_sheet(101 + sheet)
    pal = list(pal)
    if colour is not None and sheet in ACID_SHEETS:
        for slot, rgb in zip((8, 10, 12, 40), ACID[colour]):
            pal[slot] = rgb
    key = top[0][0]
    im = Image.new("RGBA", (CELL, CELL))
    for y in range(CELL):
        for x in range(CELL):
            i = top[y][cell * CELL + x]
            im.putpixel((x, y), (*pal[i], 0 if i == key else 255))
    if mirrored:
        im = im.transpose(Image.FLIP_LEFT_RIGHT)
    return im


class Sim:
    def __init__(self) -> None:
        self.x = 150
        self.base_y = 20
        self.y = self.base_y
        self.facing = 1
        self.index = 3
        self.acid = False
        self.colour = 0
        self.prop: int | None = None
        self.prop_x = 0
        self.frames: list[Image.Image] = []

    def draw_index(self) -> tuple[int, bool]:
        idx = self.index
        if self.acid and idx < 176:
            idx += 176
        if not self.acid and 9 <= idx <= 14:
            return idx, False
        return idx, self.facing <= 0

    def snap(self) -> None:
        canvas = Image.new("RGBA", (300, 80), (0, 128, 128, 255))
        if self.prop is not None:
            canvas.alpha_composite(sprite(self.prop, False, None), (self.prop_x, self.base_y))
        idx, mirrored = self.draw_index()
        canvas.alpha_composite(sprite(idx, mirrored, self.colour if self.acid else None), (self.x, self.y))
        self.frames.append(canvas.convert("RGB").resize((canvas.width * SCALE, canvas.height * SCALE), Image.NEAREST))


def run(sim: Sim) -> None:
    # 166/167: graze.
    stage, spin, counter, pos = 0, 0, 0, 0
    sim.prop_x = sim.x - CELL
    sim.prop = 352
    sim.snap()
    while True:
        if sim.prop is not None:
            spin = (spin + 1) % 8
            sim.prop = 352 + stage * 8 + spin
        if counter < 2:
            counter += 1
            sim.snap()
            continue
        counter = 0
        idx = GRAZE_FRAMES[pos]
        pos += 1
        if idx == 2:
            sim.x -= 8
            sim.index = 2
            sim.snap()
            continue
        if BITE_1 <= idx <= BITE_GONE:
            if idx == BITE_GONE:
                sim.prop = None
            else:
                stage = max(stage, idx - BITE_1 + 1)
            idx = GRAZE_FRAMES[pos]
            pos += 1
        if idx == 0:
            sim.snap()
            break
        sim.index = idx
        sim.snap()

    # 168: scared twice, every 2 ticks.
    for _ in range(2):
        for idx in AMAZED:
            sim.snap()
            if idx == 0:
                break
            sim.index = idx
            sim.snap()

    # 169: acid on.
    sim.acid = True
    sim.colour = 0
    sim.facing = 1
    ticks = 0

    def advance() -> None:
        nonlocal ticks
        ticks += 1
        if ticks % 2 == 0:
            sim.colour = (sim.colour + 1) % len(ACID)

    def roll_step(index: int) -> None:
        sim.x -= sim.facing * 8
        sim.x = max(0, min(sim.x, 300 - CELL))
        sim.index = index

    sim.snap()
    for _ in range(16):  # 170 spin
        sim.index = SPIN[ticks % 8]
        advance()
        sim.snap()
    ticks = 0
    for _ in range(16):  # 171 flip + bounce
        if ticks % 2 == 0:
            sim.facing = -sim.facing
        sim.y = sim.base_y + BOUNCE[ticks % 4]
        sim.index = 3
        advance()
        sim.snap()
    sim.y = sim.base_y
    ticks = 0
    for _ in range(12):  # 172 roll
        roll_step(ROLL[ticks % 8])
        advance()
        sim.snap()
    ticks = 0
    sim.facing = -sim.facing
    for _ in range(12):  # 173 upside-down roll
        roll_step(384 + ROLL[ticks % 8] - 112)
        advance()
        sim.snap()
    ticks = 0
    blink = 0
    while blink < 8:  # 174 dizzy
        advance()
        if ticks % 2 != 0:
            sim.snap()
            continue
        sim.index = BLINK[blink]
        blink += 1
        sim.snap()
    sim.acid = False
    sim.index = 3
    sim.snap()


def main() -> None:
    sim = Sim()
    run(sim)
    frames = sim.frames + [sim.frames[-1]] * 8
    out = ROOT / "docs" / "images" / "acid_trip_preview.gif"
    frames[0].save(out, save_all=True, append_images=frames[1:], duration=TICK_MS, loop=0)
    print(f"Wrote {out} ({len(frames)} frames, {len(frames) * TICK_MS / 1000:.1f}s)")


if __name__ == "__main__":
    main()
