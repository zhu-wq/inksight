"""Stage the legacy geometry outside the immutable ZIP driver sources."""
from pathlib import Path
from shutil import copytree


def stage(project):
    src = project / "lib/JD79665"
    dest = project / ".pio/se0398nz07-legacy-libs/JD79665Legacy"
    copytree(src, dest, dirs_exist_ok=True)
    header = dest / "src/epd4c/GxEPD2_380_JD79665.h"
    data = header.read_bytes()
    for original, legacy in ((b"WIDTH = 800;", b"WIDTH = 768;"),
                             (b"HEIGHT = 600;", b"HEIGHT = 552;")):
        if data.count(original) != 1:
            raise ValueError("Unexpected ZIP driver geometry")
        data = data.replace(original, legacy)
    header.write_bytes(data)


Import("env")
stage(Path(env.subst("$PROJECT_DIR")))
