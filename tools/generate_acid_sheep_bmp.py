"""Generate Scmpoo/125.bmp — 108.bmp upside down, for the acid trip's upside-down roll.

Pupils are split onto their own palette index (40) exactly like the alien sheets, so the
game can recolour horns (8, 10, 12) and eyes (40) at runtime.
"""
import struct
from pathlib import Path

from generate_alien_sheep_bmp import load_pixels, recolor_alien, write_bi_rgb


def main():
    root = Path(__file__).resolve().parents[1]
    src = root / "Scmpoo" / "108.bmp"
    dst = root / "Scmpoo" / "125.bmp"
    data = bytearray(src.read_bytes())
    img, pal_off, w, h, n = load_pixels(data)
    top = img[::-1]
    pal = [list(struct.unpack_from("<BBBB", data, pal_off + i * 4)) for i in range(n)]
    recolor_alien(top, pal, w, h, n)
    write_bi_rgb(dst, top[::-1], pal, w, h, n)


if __name__ == "__main__":
    main()
