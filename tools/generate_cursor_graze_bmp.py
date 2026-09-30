"""Build Scmpoo/123.bmp and 124.bmp — spinning VRCURSOR.ani prop being eaten.

Grid of 4 bite stages x 8 spin frames, 40x40 cells, sprite = 352 + stage * 8 + spin:
  123.bmp  stage 0 (full), stage 1 (~75% left)
  124.bmp  stage 2 (~50% left), stage 3 (~25% left)

Bites are taken from the top (the cursor's head) downward.

Chroma key is palette index 13 = (0,0,255), matching other sheets.
"""
from __future__ import annotations

import struct
from pathlib import Path

try:
    from PIL import Image
except ImportError as exc:  # pragma: no cover
    raise SystemExit("Pillow required: pip install pillow") from exc

CHROMA_IDX = 13
CHROMA_RGB = (0, 0, 255)
CELL = 40
SHEET_CELLS = 16


def load_ani_frames(path: Path) -> list[Image.Image]:
    data = path.read_bytes()
    i = 12
    icons: list[bytes] = []
    while i + 8 <= len(data):
        fourcc = data[i : i + 4]
        size = struct.unpack_from("<I", data, i + 4)[0]
        payload = data[i + 8 : i + 8 + size]
        if fourcc == b"LIST" and payload[:4] == b"fram":
            j = 4
            while j + 8 <= size:
                fc = payload[j : j + 4]
                sz = struct.unpack_from("<I", payload, j + 4)[0]
                if fc == b"icon":
                    icons.append(payload[j + 8 : j + 8 + sz])
                j += 8 + sz + (sz & 1)
        i += 8 + size + (size & 1)
    return [icon_to_rgba(ic) for ic in icons]


def icon_to_rgba(ic: bytes) -> Image.Image:
    width, height, _ncolors, _res, _planes, _bitcount, bytesinres, imageoffset = struct.unpack_from(
        "<BBBBHHII", ic, 6
    )
    if width == 0:
        width = 256
    if height == 0:
        height = 256
    imgdata = ic[imageoffset : imageoffset + bytesinres]
    _biSize, bw, bh, _bplanes, bbpp = struct.unpack_from("<IiiHH", imgdata, 0)
    xor_h = abs(bh) // 2
    xor_w = bw
    off = 40
    ncols = struct.unpack_from("<I", imgdata, 32)[0]
    if bbpp <= 8:
        if ncols == 0:
            ncols = 1 << bbpp
        pal = []
        for i in range(ncols):
            b, g, r, _a = imgdata[off + i * 4 : off + i * 4 + 4]
            pal.append((r, g, b))
        off += ncols * 4
        row = ((xor_w * bbpp + 31) // 32) * 4
        rgba = Image.new("RGBA", (xor_w, xor_h))
        for y in range(xor_h):
            rowdata = imgdata[off + (xor_h - 1 - y) * row : off + (xor_h - y) * row]
            for x in range(xor_w):
                if bbpp == 8:
                    idx = rowdata[x]
                elif bbpp == 4:
                    byte = rowdata[x // 2]
                    idx = (byte >> 4) & 0xF if (x % 2) == 0 else byte & 0xF
                else:
                    byte = rowdata[x // 8]
                    idx = (byte >> (7 - x % 8)) & 1
                r, g, b = pal[idx]
                rgba.putpixel((x, y), (r, g, b, 255))
        off += row * xor_h
    elif bbpp == 32:
        row = ((xor_w * 32 + 31) // 32) * 4
        rgba = Image.new("RGBA", (xor_w, xor_h))
        for y in range(xor_h):
            rowdata = imgdata[off + (xor_h - 1 - y) * row : off + (xor_h - y) * row]
            for x in range(xor_w):
                b, g, r, a = rowdata[x * 4 : x * 4 + 4]
                rgba.putpixel((x, y), (r, g, b, a))
        off += row * xor_h
    else:
        row = ((xor_w * bbpp + 31) // 32) * 4
        rgba = Image.new("RGBA", (xor_w, xor_h))
        for y in range(xor_h):
            rowdata = imgdata[off + (xor_h - 1 - y) * row : off + (xor_h - y) * row]
            for x in range(xor_w):
                if bbpp == 24:
                    b, g, r = rowdata[x * 3 : x * 3 + 3]
                else:
                    b = g = r = 0
                rgba.putpixel((x, y), (r, g, b, 255))
        off += row * xor_h

    mask_row = ((xor_w + 31) // 32) * 4
    for y in range(xor_h):
        if off + (xor_h - y) * mask_row > len(imgdata):
            break
        rowdata = imgdata[off + (xor_h - 1 - y) * mask_row : off + (xor_h - y) * mask_row]
        for x in range(xor_w):
            bit = (rowdata[x // 8] >> (7 - x % 8)) & 1
            if bit:
                r, g, b, _a = rgba.getpixel((x, y))
                rgba.putpixel((x, y), (r, g, b, 0))
    return rgba


def center_on_cell(frame: Image.Image) -> Image.Image:
    cell = Image.new("RGBA", (CELL, CELL), (*CHROMA_RGB, 0))
    x = (CELL - frame.width) // 2
    y = (CELL - frame.height) // 2
    cell.alpha_composite(frame, (x, y))
    return cell


BITE_KEEP = (1.0, 0.75, 0.5, 0.25)
SPIN_FRAMES = 8
BITE_JAG = (0, 2, 1, 3, 1, 0, 2, 1)


def opaque_y_range(cell: Image.Image) -> tuple[int, int] | None:
    ys = [y for y in range(CELL) for x in range(CELL) if cell.getpixel((x, y))[3] >= 128]
    if not ys:
        return None
    return min(ys), max(ys) + 1


def bite_from_top(cell: Image.Image, keep_ratio: float) -> Image.Image:
    """Keep the bottom keep_ratio of the cursor; the head is chewed off with a jagged edge."""
    if keep_ratio >= 1.0:
        return cell.copy()
    span = opaque_y_range(cell)
    if span is None:
        return cell.copy()
    y0, y1 = span
    cut_y = y1 - int((y1 - y0) * keep_ratio)
    out = cell.copy()
    for x in range(CELL):
        col_cut = cut_y + BITE_JAG[x % len(BITE_JAG)]
        for y in range(0, min(CELL, col_cut)):
            out.putpixel((x, y), (*CHROMA_RGB, 0))
    return out


def opaque_count(cell: Image.Image) -> int:
    return sum(1 for px in cell.getdata() if px[3] >= 128)


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
    for rgb, _count in ranked:
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
    best = 0
    best_d = 1 << 30
    for i, (pr, pg, pb) in enumerate(pal):
        if i == CHROMA_IDX:
            continue
        d = (pr - rgb[0]) ** 2 + (pg - rgb[1]) ** 2 + (pb - rgb[2]) ** 2
        if d < best_d:
            best_d = d
            best = i
    return best


def quantize_cell(cell: Image.Image, pal: list[tuple[int, int, int]]) -> list[list[int]]:
    rows: list[list[int]] = []
    for y in range(CELL):
        row = []
        for x in range(CELL):
            r, g, b, a = cell.getpixel((x, y))
            if a < 128:
                row.append(CHROMA_IDX)
            else:
                row.append(nearest_idx(pal, (r, g, b)))
        rows.append(row)
    return rows


def write_bi_rgb(dst: Path, top: list[list[int]], pal: list[tuple[int, int, int]], w: int, h: int) -> None:
    n = 256
    img_out = top[::-1]
    row_bytes = ((w * 8 + 31) // 32) * 4
    pix = bytearray()
    for y in range(h):
        pix.extend(img_out[y])
        pix.extend(b"\x00" * (row_bytes - w))
    bf_off = 14 + 40 + n * 4
    out = bytearray()
    out.extend(struct.pack("<2sIHHI", b"BM", bf_off + len(pix), 0, 0, bf_off))
    out.extend(struct.pack("<IiiHHIIiiII", 40, w, h, 1, 8, 0, len(pix), 0, 0, n, n))
    for i in range(n):
        r, g, b = pal[i]
        out.extend(struct.pack("<BBBB", b, g, r, 0))
    out.extend(pix)
    dst.write_bytes(out)
    print(f"Wrote {dst} ({len(out)} bytes, {w}x{h})")


def main() -> None:
    root = Path(__file__).resolve().parents[1]
    ani = Path(r"c:\Users\NinjaArt\VRCURSOR.ani")
    project_ani = root / "Scmpoo" / "VRCURSOR.ani"
    if not ani.is_file():
        ani = project_ani
    if not ani.is_file():
        raise SystemExit(f"ANI not found: {ani}")

    frames = load_ani_frames(ani)
    if not frames:
        raise SystemExit("No frames in ANI")

    spin = [
        center_on_cell(frames[i * len(frames) // SPIN_FRAMES]) for i in range(SPIN_FRAMES)
    ]
    grid = [[bite_from_top(c, keep) for c in spin] for keep in BITE_KEEP]

    for s in range(1, len(grid)):
        for f in range(SPIN_FRAMES):
            if opaque_count(grid[s][f]) >= opaque_count(grid[s - 1][f]):
                raise SystemExit(f"Stage {s} frame {f} is not smaller than stage {s - 1}")

    cells = [c for row in grid for c in row]
    pal = build_palette(cells)
    w = CELL * SHEET_CELLS
    h = CELL
    for sheet in range(len(cells) // SHEET_CELLS):
        top = [[CHROMA_IDX] * w for _ in range(h)]
        for ci, cell in enumerate(cells[sheet * SHEET_CELLS : (sheet + 1) * SHEET_CELLS]):
            q = quantize_cell(cell, pal)
            x0 = ci * CELL
            for y in range(CELL):
                for x in range(CELL):
                    top[y][x0 + x] = q[y][x]
        # The loader builds the mask from the first pixel's colour.
        top[0][0] = CHROMA_IDX
        write_bi_rgb(root / "Scmpoo" / f"{123 + sheet}.bmp", top, pal, w, h)

    preview = Image.new("RGB", (CELL * SPIN_FRAMES, CELL * len(BITE_KEEP)), CHROMA_RGB)
    for s, row in enumerate(grid):
        for f, cell in enumerate(row):
            preview.paste(cell, (f * CELL, s * CELL), cell)
    preview_path = root / "docs" / "images" / "cursor_graze_grid.png"
    preview_path.parent.mkdir(parents=True, exist_ok=True)
    preview.resize((preview.width * 3, preview.height * 3), Image.NEAREST).save(preview_path)
    print(f"Wrote {preview_path}")

    if ani.resolve() != project_ani.resolve():
        project_ani.write_bytes(ani.read_bytes())
        print(f"Copied ANI to {project_ani}")


if __name__ == "__main__":
    main()
