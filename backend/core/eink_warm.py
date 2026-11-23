"""Four-ink photo preparation, independently implemented with array operations.

Reference presets: MoInk 343be94, natural and adaptive (sunset). Output is RGB;
the renderer owns the device palette, dithering and frame packing.
"""
import numpy as np
from PIL import Image

_LUMA = (0.2126, 0.7152, 0.0722)
_STOPS = (0, .10, .26, .42, .56, .70, .84, .93, 1)
_SUNSET = ((0, 0, 0), (26, 6, 4), (92, 18, 12), (168, 40, 16),
           (214, 86, 18), (236, 140, 26), (248, 200, 60),
           (255, 235, 150), (255, 255, 255))


def _prepare(rgb):
    """Shared luminance-percentile levels and edge-replicating unsharp filter."""
    y = np.floor(rgb @ _LUMA + .5).astype(np.uint8)
    counts = np.bincount(y.ravel(), minlength=256).cumsum()
    lo, hi = counts.searchsorted(np.array([.005, .995]) * y.size)
    if hi - lo >= 8:
        rgb = np.clip(18 + (rgb - lo) * (228 / (hi - lo)), 0, 255)
    blurred = rgb
    for axis in (0, 1):
        p = [(0, 0)] * 3
        p[axis] = (1, 1)
        rows = np.pad(blurred, p, mode="edge").swapaxes(axis, 0)
        blurred = ((rows[:-2] + 2 * rows[1:-1] + rows[2:]) / 4).swapaxes(0, axis)
    return np.clip(rgb + .30 * (rgb - blurred), 0, 255) / 255


def _from_hsl(hue, saturation, light):
    # Triangle-wave form of the HSL inverse; hue is in degrees.
    phase = (hue[..., None] / 30 + np.array([0, 8, 4])) % 12
    triangle = np.clip(np.minimum(phase - 3, 9 - phase), -1, 1)
    return light[..., None] - (saturation * np.minimum(light, 1 - light))[..., None] * triangle


def map_photo(image: Image.Image, preset: str) -> Image.Image:
    rgb = _prepare(np.asarray(image.convert("RGB"), dtype=np.float64))
    high, low = rgb.max(axis=2), rgb.min(axis=2)
    light, chroma = (high + low) / 2, high - low
    r, g, b = rgb.transpose(2, 0, 1)
    span = np.where(chroma > 0, chroma, 1)
    hue = (60 * np.select([high == r, high == g],
                          [(g - b) / span, 2 + (b - r) / span],
                          default=4 + (r - g) / span)) % 360
    sat = np.clip(chroma / np.maximum(1e-12, 1 - abs(2 * light - 1)), 0, 1)

    if preset == "natural":
        t = np.maximum(np.clip((110 - hue) / 50, 0, 1),
                       np.clip((hue - 300) / 50, 0, 1))
        warm = t * t * (3 - 2 * t)
        saturation = sat ** (1 - .45 * warm) * (.75 + .25 * warm)
        shift = np.clip(-.25 * (1 - warm) * (hue - 60), -60, 0)
        shift += warm * np.clip(.12 * ((210 - hue) % 360 - 180), -18, 18)
        mapped = _from_hsl(hue + shift, saturation, light)
    elif preset == "adaptive":
        mapped = _from_hsl(1 + hue * 58 / 360, sat, light)
        luminance = np.floor(np.clip(mapped @ _LUMA, 0, 1) * 255 + .5) / 255
        ramp = np.stack([np.interp(luminance, _STOPS, channel)
                         for channel in zip(*_SUNSET)], axis=-1) / 255
        mapped = .6 * mapped + .4 * ramp
    else:
        raise ValueError(f"Unknown eink_warm preset: {preset}")
    return Image.fromarray(np.floor(np.clip(mapped, 0, 1) * 255 + .5).astype(np.uint8))
