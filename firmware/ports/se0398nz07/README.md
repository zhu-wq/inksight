# SE0398NZ07 3.98 寸四色屏 · ESP32-S3 N16R8

基于[创客的 Nova 14 手机壳屏适配](https://makerworld.com.cn/zh/models/2399226)，此目录是独立 PlatformIO 工程。后台画布始终为 768×552。根据排线版本与批次选构建目标：

| 环境 | 驱动尺寸 | 说明 |
|---|---|---|
| `epd_398_bwry_s3_n16r8_A0` | 768×552 | 创客原方案，无偏移 |
| `epd_398_bwry_s3_n16r8_A1_old` | 768×552 | 立创附件的 GxEPD2 JD79665 驱动，使用旧批次尺寸，无偏移 |
| `epd_398_bwry_s3_n16r8_a1_new` | 800×600 | 默认；同一附件驱动，使用原始 RAM 尺寸，图像从 X=32 写入 |

A0 使用创客 ZIP 原始 `src/epd_driver.cpp`。A1 使用[立创点屏工程](https://oshwhub.com/chen7654/project_rarhcffq)的 `ESP32_3in98_Eink.zip`：仓库内 `GxEPD2_380_JD79665.h` 和 `.cpp` 与附件逐字节相同。`GxEPD2.h` 在 `Panel` 枚举末尾注册 `GDEM0380F52`，`GxEPD2_4C.h` 引入该面板头文件并转发 `writeImage2bpp()`。[krstc](https://github.com/krstc/InkSight_adapt_HUAWEI_eink/tree/78ceea6) 仅作参考。

新批次直接编译原驱动：800×600，每行 200 字节，整帧 120,000 字节。后台的 768×552 图像从 `(32,0)` 写入，其余补白。附件将尺寸写死在头文件中；仅旧批次构建时，`prepare_a1_old.py` 在 `.pio/` 下生成副本，将两个尺寸常量换回 768×552，使用 `(0,0)`，不改仓库内的原文件。两种配置共用下方接线，不使用作者定制板的 GPIO41 电源开关。

从仓库根目录构建，例如：

```sh
pio run -d firmware/ports/se0398nz07 -e epd_398_bwry_s3_n16r8_A1_old
```

SPI 驱动转接板接线：DIN→GPIO11，CLK→GPIO12，CS→GPIO10，DC→GPIO9，RST→GPIO8，BUSY→GPIO7，GND 共地。供电按转接板规格；本表不适用于裸屏排线。Flash 为 16 MB，PSRAM 为 8 MB OPI。移植未进行实屏验证。

参考：[A1 批次与 JD79665 驱动说明](https://oshwhub.com/chen7654/project_rarhcffq)。
