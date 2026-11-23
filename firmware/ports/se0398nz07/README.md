# SE0398NZ07 3.98 寸四色屏 · ESP32-S3 N16R8

基于[创客的 Nova 14 手机壳屏适配](https://makerworld.com.cn/zh/models/2399226)，此目录是独立 PlatformIO 工程。后台画布始终为 768×552。根据排线版本与批次选构建目标：

| 环境 | 驱动尺寸 | 说明 |
|---|---|---|
| `epd_398_bwry_s3_n16r8_A0` | 768×552 | 创客原方案，无偏移 |
| `epd_398_bwry_s3_n16r8_A1_old` | 768×552 | 立创附件的 GxEPD2 JD79665 驱动，使用旧批次尺寸，无偏移 |
| `epd_398_bwry_s3_n16r8_a1_new` | 800×600 | 默认；同一附件驱动，使用原始 RAM 尺寸，图像从 X=32 写入 |

A0 单独编译创客 ZIP 原始 `src/epd_driver.cpp`。两个 A1 环境都通过 `src/jd79665_a1.cpp` 调用本地 `lib/JD79665`：这是从[立创点屏工程](https://oshwhub.com/chen7654/project_rarhcffq)附件 `ESP32_3in98_Eink.zip` 提取的面板类和 GxEPD2 基类（附原许可证）。本地改动是选择批次尺寸和记录上电状态，以便刷完后正常下电。[krstc](https://github.com/krstc/InkSight_adapt_HUAWEI_eink/tree/78ceea6) 仅作协议参考。新批次每行先发送 8 个白色字节，再发送 192 字节可见内容，最后 48 行补白；后台仍使用 768×552。SPI 使用下表引脚，不使用示例定制板的 GPIO41 电源开关。

从仓库根目录构建，例如：

```sh
pio run -d firmware/ports/se0398nz07 -e epd_398_bwry_s3_n16r8_A1_old
```

SPI 驱动转接板接线：DIN→GPIO11，CLK→GPIO12，CS→GPIO10，DC→GPIO9，RST→GPIO8，BUSY→GPIO7，GND 共地。供电按转接板规格；本表不适用于裸屏排线。Flash 为 16 MB，PSRAM 为 8 MB OPI。移植未进行实屏验证。

参考：[A1 批次与 JD79665 驱动说明](https://oshwhub.com/chen7654/project_rarhcffq)。
