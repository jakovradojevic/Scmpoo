"""Install Scmpoo/111.wav — mushrooms acid-trip trance bed.

Source (CC0 / public domain, no attribution required):
  "TR LOOP 04.wav" by zgump on Freesound
  https://freesound.org/people/zgump/sounds/72584/
  Pack: TRANCE LOOP PACK 01

Preview is fetched, converted to mono 16-bit PCM 22050 Hz (game format), and peak-normalized.
Re-run this script if 111.wav needs regenerating (needs network + ffmpeg).
"""
from __future__ import annotations

import subprocess
import urllib.request
import wave
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "Scmpoo" / "111.wav"
CACHE = ROOT / "tools" / "_trance_preview.mp3"
PREVIEW_URL = "https://cdn.freesound.org/previews/72/72584_377011-hq.mp3"
SR = 22050


def main() -> None:
    if not CACHE.exists():
        print(f"download {PREVIEW_URL}")
        urllib.request.urlretrieve(PREVIEW_URL, CACHE)
    tmp = CACHE.with_suffix(".wav")
    subprocess.check_call(
        [
            "ffmpeg",
            "-y",
            "-i",
            str(CACHE),
            "-ac",
            "1",
            "-ar",
            str(SR),
            "-sample_fmt",
            "s16",
            str(tmp),
        ],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )
    with wave.open(str(tmp), "rb") as w:
        x = np.frombuffer(w.readframes(w.getnframes()), dtype=np.int16).astype(np.float64)
    peak = float(np.max(np.abs(x)) or 1.0)
    x = (x / peak * 0.92 * 32767.0).astype(np.int16)
    OUT.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(OUT), "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(SR)
        w.writeframes(x.tobytes())
    tmp.unlink(missing_ok=True)
    print(f"wrote {OUT} ({len(x) / SR:.2f}s, {OUT.stat().st_size} bytes)")
    print("source: freesound.org/people/zgump/sounds/72584/ (CC0)")


if __name__ == "__main__":
    main()
