# SE0398NZ07 3.98 寸四色屏 · ESP32-S3 N16R8

## 先选屏幕批次，再选构建目标

本目录是独立 PlatformIO 工程，三个目标板均为 **ESP32-S3-WROOM-1-N16R8（16 MB Flash、8 MB OPI PSRAM）**。S2、C3 是参考工程的主控。`A1_old` 和 `a1_new` 是本项目用于区分 A1 新旧批次的名字，排线上不一定印有 `old/new`。三套后台画布都是 **768×552、2 bit/像素**；区别在设备端。

| 环境 | 驱动尺寸 | 说明 |
|---|---|---|
| `epd_398_bwry_s3_n16r8_A0` | 768×552 | 创客原方案，无偏移 |
| `epd_398_bwry_s3_n16r8_A1_old` | 768×552 | 立创附件的 GxEPD2 JD79665 驱动，使用旧批次尺寸，无偏移 |
| `epd_398_bwry_s3_n16r8_a1_new` | 800×600 | 默认；同一附件驱动，使用原始 RAM 尺寸，图像从 X=32 写入 |

### 第一套：SE0398NZ07-FNG-A0，S2 + PSRAM → S3 N16R8

原始依据是[创客项目](https://makerworld.com.cn/zh/models/2399226)的 `inksight-mainbwry-s2-HW3.98四色源码.zip`。**以下行号指原 ZIP，不是当前仓库根目录的同名文件**：

- `firmware/src/epd_driver.cpp:258–261` 写明「3.98寸552x768四色墨水屏」，快速初始化分支调用 `epdInit()`。
- `firmware/platformio.ini:345–362` 的 `epd_75_jdy79668_s2_psram` 使用 S2、PSRAM、`768×552`、`EPD_BPP=2`。`epd_75` 是沿用的名称，不代表此目标实际驱动 7.5 寸屏。
- 本移植通过 S3 配置编译 [A0 原驱动](src/epd_driver.cpp)，使用软件 SPI 和创客的逐行映射。维护 A0 时，不改 A1 的 GxEPD2 面板类。

### 第二套：SE0398NZ07-FNG-A1 旧批次，C3 参考 → S3 N16R8

IC 为 **JD79665**。[krstc `78ceea6` 完整工程](https://github.com/krstc/InkSight_adapt_HUAWEI_eink/tree/78ceea65ec754fcacc3a9a6036416b755bd2f64c)是 **C3 + A1 的整机功能参考**；[立创的 `ESP32_3in98_Eink.zip`](https://oshwhub.com/chen7654/project_rarhcffq)是 **S3 点屏示例和面板驱动来源**，不是一套完整的 InkSight 应用。当前 S3 移植的应用代码来自创客工程，实际点屏走 [A1 接口](src/jd79665_a1.cpp) 和硬件 SPI，不编译 krstc 的 C3 驱动。

旧批次的可见区和驱动 RAM 都为 **768×552**，从 `(0,0)` 写入。立创附件头文件的默认尺寸是 800×600，所以只有旧批次运行 [`prepare_a1_old.py`](prepare_a1_old.py)，在 `.pio/` 创建 768×552 的构建副本。要调整旧批次尺寸，改该脚本；不要编辑生成的副本或立创原文件。

### 第三套：SE0398NZ07-FNG-A1 新批次，沿用第二套 A1 参考 → S3 N16R8

IC 同为 **JD79665**，与第二套共用 A1 接口、颜色编码和接线。C3 是完整工程的参考起点；**新批次消除中缝的方法来自立创 S3 点屏工程，不是 krstc 已验证的新批次 C3 配置**。

新批次用附件原始尺寸初始化 **800×600**（200 字节/行，共 120,000 字节）。将后台 **768×552** 图像从 **`(32,0)`** 写入，左侧 32 像素和底部 48 行补白。不要把后台画布一起改为 800×600，也不要把这个偏移套用到旧批次。

## 接线：按主控列读取 GPIO

以下是**屏幕驱动转接板**的 SPI 信号，不是裸屏 FPC 脚序。前三套 S3 构建均使用表中 **S3** 一列；右边两列只用于对照原参考工程。

| 转接板信号 | 固件宏 | **本项目 S3（三套相同）** | 创客 S2（A0 参考） | krstc C3 Pro mini（A1 参考） |
|---|---|---:|---:|---:|
| DIN / MOSI | `PIN_EPD_MOSI` | **11** | 11 | 6 |
| CLK / SCK | `PIN_EPD_SCK` | **12** | 7 | 4 |
| CS | `PIN_EPD_CS` | **10** | 12 | 7 |
| D/C | `PIN_EPD_DC` | **9** | 17 | 1 |
| RST | `PIN_EPD_RST` | **8** | 16 | 2 |
| BUSY（主控输入） | `PIN_EPD_BUSY` | **7** | 4 | 10 |

GND 共地，VCC 按转接板规格供电。立创定制板示例另用 `CS=4、DC=5、RST=6、BUSY=7`，并用 GPIO41 控制外设电源；这些不是本项目 S3 的默认接线。若实际板子改线，修改本目录 [`src/config.h`](src/config.h) 的 `BOARD_PROFILE_ESP32_S3_N16R8` 分支；这会同时影响三个 S3 目标。

## 构建与维护入口

从本目录的 [`platformio.ini`](platformio.ini) 选择目标：A0 排除 `jd79665_a1.cpp`，两个 A1 都排除 `epd_driver.cpp`。A1 共用的 [Panel ID 注册](lib/JD79665/src/GxEPD2.h) 和 [四色模板接入](lib/JD79665/src/GxEPD2_4C.h) 在库层；[`GxEPD2_380_JD79665.h/.cpp`](lib/JD79665/src/epd4c/) 保留附件原件。改 A1 共用接口后需同时检查新旧两套；只改批次行为时要选对应环境。

从仓库根目录构建，例如：

```sh
pio run -d firmware/ports/se0398nz07 -e epd_398_bwry_s3_n16r8_A1_old
```

验证状态：A1 两套已做模拟 SPI 数据检查；本移植尚未进行实屏验证。

参考：[A1 批次与 JD79665 驱动说明](https://oshwhub.com/chen7654/project_rarhcffq)。
