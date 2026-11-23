# SE0398NZ07 3.98 寸四色屏 · ESP32-S3 N16R8

基于[创客的 Nova 14 手机壳屏适配](https://makerworld.com.cn/zh/models/2399226)，此目录是独立 PlatformIO 工程。后台画布始终为 768×552。根据排线版本与批次选构建目标：

| 环境 | 驱动尺寸 | 说明 |
|---|---|---|
| `epd_398_bwry_s3_n16r8_A0` | 768×552 | 创客原方案，无偏移 |
| `epd_398_bwry_s3_n16r8_A1_old` | 768×552 | A1 旧批次，无偏移 |
| `epd_398_bwry_s3_n16r8_a1_new` | 800×600 | 默认；A1 新批次，X 偏移 +32 |

从仓库根目录构建，例如：

```sh
pio run -d firmware/ports/se0398nz07 -e epd_398_bwry_s3_n16r8_A1_old
```

SPI 驱动转接板接线：DIN→GPIO11，CLK→GPIO12，CS→GPIO10，DC→GPIO9，RST→GPIO8，BUSY→GPIO7，GND 共地。供电按转接板规格；本表不适用于裸屏排线。Flash 为 16 MB，PSRAM 为 8 MB OPI。移植未进行实屏验证。

参考：[A1 批次与 JD79665 驱动说明](https://oshwhub.com/chen7654/project_rarhcffq)。
