Import("env")
import os
import subprocess

def merge_bin(source, target, env):
    firmware = str(target[0])
    board = env.BoardConfig()
    mcu = board.get("build.mcu", "esp32")
    flash_mode = "dio"
    flash_freq = board.get("build.f_flash", "40000000L").replace("L", "")
    flash_freq_m = str(int(int(flash_freq) / 1000000)) + "m"
    flash_size = board.get("upload.flash_size", "4MB")

    platform = env.PioPlatform()
    native_tool = platform.get_package_dir("tool-esptool")
    launcher = ([os.path.join(native_tool, "esptool.exe" if os.name == "nt" else "esptool")]
                if native_tool else [env.subst("$PYTHONEXE"),
                os.path.join(platform.get_package_dir("tool-esptoolpy"), "esptool.py")])
    output = os.path.join(env.subst("$BUILD_DIR"), "firmware_merged.bin")

    extra = env.get("FLASH_EXTRA_IMAGES", [])
    print("=== FLASH_EXTRA_IMAGES ===")
    for offset, path in extra:
        print(f"  {offset} -> {env.subst(path)}")
    print(f"  APP -> 0x10000 {firmware}")
    print("==========================")

    cmd = launcher + [
        "--chip", mcu,
        "merge_bin",
        "--flash_mode", flash_mode,
        "--flash_freq", flash_freq_m,
        "--flash_size", flash_size,
        "-o", output,
    ]

    for offset, path in extra:
        cmd.extend([offset, env.subst(path)])

    cmd.extend(["0x10000", firmware])

    subprocess.run(cmd, check=True)

env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", merge_bin)
