"""Generate full alien sheep bitmap set with green horns + red eyes.

Outputs uncompressed BI_RGB 8bpp BMPs for the game loader:
  101.bmp..111.bmp -> 112.bmp..122.bmp

Fixes red silhouette outline by restoring black (idx 14) on any red
eye pixel (idx 40) that touches chroma blue (idx 13).
"""
import struct
from pathlib import Path

BLACK = 14
CHROMA = 13
RED_IDX = 40
SCLERA = {0, 16}


def load_pixels(data):
    dib = struct.unpack_from("<I", data, 14)[0]
    w, h = struct.unpack_from("<ii", data, 18)
    h = abs(h)
    n = struct.unpack_from("<I", data, 46)[0] or 256
    comp = struct.unpack_from("<I", data, 30)[0]
    pal_off = 14 + dib
    pix_off = struct.unpack_from("<I", data, 10)[0]
    img = [[0] * w for _ in range(h)]
    if comp == 0:
        row = ((w * 8 + 31) // 32) * 4
        for y in range(h):
            start = pix_off + y * row
            img[y] = list(data[start : start + w])
    elif comp == 1:
        x = y = 0
        p = pix_off
        while p + 1 < len(data) and y < h:
            a, b = data[p], data[p + 1]
            p += 2
            if a == 0:
                if b == 0:
                    x = 0
                    y += 1
                elif b == 1:
                    break
                elif b == 2:
                    x += data[p]
                    y += data[p + 1]
                    p += 2
                else:
                    for i in range(b):
                        if x < w and y < h:
                            img[y][x] = data[p + i]
                        x += 1
                    p += b + (b & 1)
            else:
                for i in range(a):
                    if x < w and y < h:
                        img[y][x] = b
                    x += 1
    else:
        raise ValueError(f"unsupported compression {comp}")
    return img, pal_off, w, h, n


def recolor_alien(top, pal, w, h, n):
    pal[8] = [0, 220, 80, 0]
    pal[10] = [0, 191, 0, 0]
    pal[12] = [0, 84, 0, 0]
    if RED_IDX >= n:
        raise ValueError("palette too small for red eye index")
    pal[RED_IDX] = [0, 0, 220, 0]

    cells = max(1, w // 40)
    for cell in range(cells):
        x0 = cell * 40
        sclera = {
            (x, y)
            for y in range(min(40, h))
            for x in range(40)
            if x0 + x < w and top[y][x0 + x] in SCLERA
        }
        if not sclera:
            continue

        # Black pixels with enough sclera neighbors become red pupils.
        for y in range(min(40, h)):
            for x in range(40):
                if top[y][x0 + x] != BLACK:
                    continue
                near = 0
                for dy in (-1, 0, 1):
                    for dx in (-1, 0, 1):
                        if dx == 0 and dy == 0:
                            continue
                        nx, ny = x + dx, y + dy
                        if 0 <= nx < 40 and 0 <= ny < min(40, h) and (nx, ny) in sclera:
                            near += 1
                if near >= 2:
                    top[y][x0 + x] = RED_IDX

        # Grow pupils one step into remaining black touching red+sclera.
        grow = []
        for y in range(min(40, h)):
            for x in range(40):
                if top[y][x0 + x] != BLACK:
                    continue
                touch_red = False
                touch_sclera = False
                for dy in (-1, 0, 1):
                    for dx in (-1, 0, 1):
                        nx, ny = x + dx, y + dy
                        if not (0 <= nx < 40 and 0 <= ny < min(40, h)):
                            continue
                        v = top[ny][x0 + nx]
                        if v == RED_IDX:
                            touch_red = True
                        if v in SCLERA:
                            touch_sclera = True
                if touch_red and touch_sclera:
                    grow.append((x, y))
        for x, y in grow:
            top[y][x0 + x] = RED_IDX

    # Restore black outline: red touching chroma blue becomes black.
    for y in range(h):
        for x in range(w):
            if top[y][x] != RED_IDX:
                continue
            edge = False
            for dy in (-1, 0, 1):
                for dx in (-1, 0, 1):
                    nx, ny = x + dx, y + dy
                    if 0 <= nx < w and 0 <= ny < h and top[ny][nx] == CHROMA:
                        edge = True
            if edge:
                top[y][x] = BLACK


def write_bi_rgb(dst, top, pal, w, h, n):
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
        out.extend(struct.pack("<BBBB", *pal[i]))
    out.extend(pix)
    dst.write_bytes(out)
    print(f"Wrote {dst} ({len(out)} bytes, BI_RGB)")


def convert(src: Path, dst: Path):
    data = bytearray(src.read_bytes())
    img, pal_off, w, h, n = load_pixels(data)
    top = img[::-1]
    pal = [list(struct.unpack_from("<BBBB", data, pal_off + i * 4)) for i in range(n)]
    recolor_alien(top, pal, w, h, n)
    write_bi_rgb(dst, top, pal, w, h, n)


def main():
    root = Path(__file__).resolve().parents[1]
    src_dir = root / "Scmpoo"
    for i in range(11):
        convert(src_dir / f"{101 + i}.bmp", src_dir / f"{112 + i}.bmp")


if __name__ == "__main__":
    main()
