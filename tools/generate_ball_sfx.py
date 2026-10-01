"""Generate ball SFX WAVs for Scmpoo.

112.wav — football kick / thump
113.wav — basketball bounce / hit
114.wav — ball pop / burst (lifetime end)

Mono 16-bit PCM @ 22050 Hz (matches 111.wav / sndPlaySound MEMORY).
"""
from __future__ import annotations

import wave
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parents[1]
SR = 22050


def write_wav(path: Path, samples: np.ndarray) -> None:
    peak = float(np.max(np.abs(samples)) or 1.0)
    pcm = (samples / peak * 0.92 * 32767.0).astype(np.int16)
    with wave.open(str(path), "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(SR)
        w.writeframes(pcm.tobytes())
    print(f"Wrote {path} ({len(pcm) / SR:.3f}s)")


def football_thump() -> np.ndarray:
    n = int(0.20 * SR)
    t = np.arange(n) / SR
    body = np.sin(2 * np.pi * (110 * np.exp(-t * 20) + 48) * t) * np.exp(-t * 16)
    noise = np.random.randn(n) * np.exp(-t * 38) * 0.45
    click = np.sin(2 * np.pi * 380 * t) * np.exp(-t * 55) * 0.28
    return body + noise + click


def basketball_bounce() -> np.ndarray:
    n = int(0.24 * SR)
    t = np.arange(n) / SR
    freq = 300 * np.exp(-t * 12) + 85
    phase = 2 * np.pi * np.cumsum(freq) / SR
    body = np.sin(phase) * np.exp(-t * 9)
    slap = np.diff(np.random.randn(n), prepend=0.0) * np.exp(-t * 32) * 0.32
    return 0.9 * body + slap


def ball_pop() -> np.ndarray:
    """Rubber balloon burst: sharp transient + air hiss + brief pitch dive."""
    n = int(0.45 * SR)
    t = np.arange(n) / SR
    # crack / pop transient
    crack = np.diff(np.random.randn(n), prepend=0.0)
    crack *= np.exp(-t * 55) * 1.4
    # broadband burst
    noise = np.random.randn(n) * np.exp(-t * 14) * 0.7
    # deflating pitch whistle
    freq = 900 * np.exp(-t * 8) + 120
    phase = 2 * np.pi * np.cumsum(freq) / SR
    whistle = np.sin(phase) * np.exp(-t * 7) * 0.35
    # low thump of air pressure release
    thump = np.sin(2 * np.pi * (70 * np.exp(-t * 25) + 40) * t) * np.exp(-t * 12) * 0.55
    return crack + noise + whistle + thump


def main() -> None:
    out = ROOT / "Scmpoo"
    np.random.seed(42)
    write_wav(out / "112.wav", football_thump())
    np.random.seed(7)
    write_wav(out / "113.wav", basketball_bounce())
    np.random.seed(99)
    write_wav(out / "114.wav", ball_pop())


if __name__ == "__main__":
    main()
