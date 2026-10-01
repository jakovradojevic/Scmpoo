"""Build Scmpoo/126.bmp + 127.bmp from online ball animation sheets.

Football: Mustitz "Soccer Ball Animation Sprites" (CC0) on OpenGameArt
  https://opengameart.org/content/soccer-ball-animation-sprites-and-3d-texture
  File: ball.all_.png (8x8 of 128px). Uses one equator spin row.

Basketball: sphere-mapped from Paul Bourke basketball equirectangular texture
  https://paulbourke.org/geometry/spherical/ (basketball_sph.png),
  rendered with mustitz/ballgen into basketball.all.png (8x8 of 128px).
  Cached under tools/_ball_assets/; regenerate with ballgen if missing.

Output: 16-cell BI_RGB 8bpp sheets, chroma index 13 = (0,0,255).
"""
from __future__ import annotations

import struct
import urllib.request
from pathlib import Path

from PIL import Image

CHROMA_IDX = 13
CHROMA_RGB = (0, 0, 255)
CELL = 40
SHEET_CELLS = 16
SPIN = 8
ROOT = Path(__file__).resolve().parents[1]
CACHE = ROOT / "tools" / "_ball_assets"
SOCCER_URL = "https://opengameart.org/sites/default/files/ball.all_.png"
SOCCER_ROW = 3
BASKET_SHEET = CACHE / "basketball.all.png"
BASKET_TEX_URL = "https://paulbourke.org/geometry/spherical/basketball_sph.png"
BASKET_ROW = 0  # clear horizontal seam spin


def build_palette(images: list[Image.Image]) -> list[tuple[int, int, int]]:
    colors: dict[tuple[int, int, int], int] = {}
    for im in images:
        for r, g, b, a in im.getdata():
            if a < 128:
                continue
            colors[(r, g, b)] = colors.get((r, g, b), 0) + 1
    ranked = sorted(colors.items(), key=lambda kv: -kv[1])
    pal = [(0, 0, 0)] * 256
    pal[CHROMA_IDX] = CHROMA_RGB
    slot = 0
    for rgb, _ in ranked:
        if rgb == CHROMA_RGB:
            continue
        while slot == CHROMA_IDX:
            slot += 1
        if slot >= 256:
            break
        pal[slot] = rgb
        slot += 1
    return pal


def nearest_idx(pal: list[tuple[int, int, int]], rgb: tuple[int, int, int]) -> int:
    if rgb == CHROMA_RGB:
        return CHROMA_IDX
    best, best_d = 0, 1 << 30
    for i, (pr, pg, pb) in enumerate(pal):
        if i == CHROMA_IDX:
            continue
        d = (pr - rgb[0]) ** 2 + (pg - rgb[1]) ** 2 + (pb - rgb[2]) ** 2
        if d < best_d:
            best_d, best = d, i
    return best


def write_bi_rgb(dst: Path, cells: list[Image.Image], pal: list[tuple[int, int, int]]) -> None:
    w, h = CELL * SHEET_CELLS, CELL
    top = [[CHROMA_IDX] * w for _ in range(h)]
    for ci, cell in enumerate(cells):
        x0 = ci * CELL
        for y in range(CELL):
            for x in range(CELL):
                r, g, b, a = cell.getpixel((x, y))
                top[y][x0 + x] = CHROMA_IDX if a < 128 else nearest_idx(pal, (r, g, b))
    top[0][0] = CHROMA_IDX
    img_out = top[::-1]
    row_bytes = ((w * 8 + 31) // 32) * 4
    pix = bytearray()
    for y in range(h):
        pix.extend(img_out[y])
        pix.extend(b"\x00" * (row_bytes - w))
    n = 256
    bf_off = 14 + 40 + n * 4
    out = bytearray()
    out.extend(struct.pack("<2sIHHI", b"BM", bf_off + len(pix), 0, 0, bf_off))
    out.extend(struct.pack("<IiiHHIIiiII", 40, w, h, 1, 8, 0, len(pix), 0, 0, n, n))
    for i in range(n):
        r, g, b = pal[i]
        out.extend(struct.pack("<BBBB", b, g, r, 0))
    out.extend(pix)
    dst.write_bytes(out)
    print(f"Wrote {dst} ({len(out)} bytes)")


def to_cell(im: Image.Image) -> Image.Image:
    """Fit ball into 40x40 with chroma padding and hard alpha."""
    im = im.convert("RGBA").copy()
    px = im.load()
    for y in range(im.height):
        for x in range(im.width):
            r, g, b, a = px[x, y]
            if a < 16:
                px[x, y] = (*CHROMA_RGB, 0)
            elif a < 200 and r > 230 and g > 230 and b > 230:
                px[x, y] = (*CHROMA_RGB, 0)
    bbox = im.split()[-1].getbbox()
    if bbox:
        im = im.crop(bbox)
    target = 34
    im = im.resize((target, target), Image.Resampling.LANCZOS)
    cell = Image.new("RGBA", (CELL, CELL), (*CHROMA_RGB, 0))
    ox = (CELL - target) // 2
    oy = (CELL - target) // 2
    cell.alpha_composite(im, (ox, oy))
    out = cell.copy()
    px = out.load()
    for y in range(CELL):
        for x in range(CELL):
            r, g, b, a = px[x, y]
            if a < 128:
                px[x, y] = (*CHROMA_RGB, 0)
            else:
                px[x, y] = (r, g, b, 255)
    return out


def frames_from_sheet(path: Path, row: int) -> list[Image.Image]:
    sheet = Image.open(path).convert("RGBA")
    cell = sheet.width // 8
    assert sheet.width == cell * 8 and sheet.height == cell * 8, path
    frames = []
    for col in range(SPIN):
        fr = sheet.crop((col * cell, row * cell, (col + 1) * cell, (row + 1) * cell))
        frames.append(to_cell(fr))
    return frames


def load_soccer_frames() -> list[Image.Image]:
    CACHE.mkdir(parents=True, exist_ok=True)
    path = CACHE / "ball.all_.png"
    if not path.is_file():
        print(f"download {SOCCER_URL}")
        urllib.request.urlretrieve(SOCCER_URL, path)
    return frames_from_sheet(path, SOCCER_ROW)


def ensure_basketball_sheet() -> Path:
    """Use cached ballgen sheet; rebuild from Bourke texture if missing."""
    CACHE.mkdir(parents=True, exist_ok=True)
    if BASKET_SHEET.is_file():
        return BASKET_SHEET

    tex = CACHE / "basketball_sph.png"
    if not tex.is_file():
        print(f"download {BASKET_TEX_URL}")
        urllib.request.urlretrieve(BASKET_TEX_URL, tex)

    try:
        from make_sprites_local import run_ballgen  # type: ignore
    except ImportError:
        pass

    # Prefer a checked-in / prebuilt sheet; otherwise call ballgen if available.
    ballgen = Path("/tmp/ball_assets/ballgen/make_sprites.py")
    if not ballgen.is_file():
        ballgen = CACHE / "ballgen" / "make_sprites.py"
    if ballgen.is_file():
        import subprocess
        import sys

        out_prefix = CACHE / "basketball"
        print("rendering basketball sheet via ballgen (slow)...")
        subprocess.check_call(
            [
                sys.executable,
                str(ballgen),
                str(tex),
                "8",
                "-s",
                "128x128",
                "-d",
                "96",
                "-o",
                str(out_prefix),
            ]
        )
        generated = Path(str(out_prefix) + ".all.png")
        if generated.is_file():
            generated.replace(BASKET_SHEET)
            return BASKET_SHEET

    raise FileNotFoundError(
        f"Missing {BASKET_SHEET}. Place basketball.all.png (8x8@128) under tools/_ball_assets/"
    )


def load_basketball_frames() -> list[Image.Image]:
    return frames_from_sheet(ensure_basketball_sheet(), BASKET_ROW)


def main() -> None:
    soccer = load_soccer_frames()
    basket = load_basketball_frames()
    empty = Image.new("RGBA", (CELL, CELL), (*CHROMA_RGB, 0))
    write_bi_rgb(ROOT / "Scmpoo" / "126.bmp", soccer + [empty] * (SHEET_CELLS - SPIN), build_palette(soccer))
    write_bi_rgb(ROOT / "Scmpoo" / "127.bmp", basket + [empty] * (SHEET_CELLS - SPIN), build_palette(basket))

    prev = ROOT / "docs" / "images"
    prev.mkdir(parents=True, exist_ok=True)
    for name, frames in (("126", soccer), ("127", basket)):
        strip = Image.new("RGBA", (CELL * SPIN, CELL), (*CHROMA_RGB, 255))
        for i, fr in enumerate(frames):
            strip.paste(fr, (i * CELL, 0), fr)
        strip.resize((CELL * SPIN * 4, CELL * 4), Image.Resampling.NEAREST).save(
            prev / f"ball_{name}_preview.png"
        )
        print("preview", name)


if __name__ == "__main__":
    main()
