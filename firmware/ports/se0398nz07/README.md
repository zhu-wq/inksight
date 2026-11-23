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

验证状态：三个 S3 目标均已在 macOS 使用 PlatformIO Espressif32 8.0.0 / Arduino-ESP32 3.3.12 完成编译和固件合并，产物位于本工程的 `.pio/build/<环境名>/firmware_merged.bin`。A1 两套另已做模拟 SPI 数据检查；尚未进行实屏验证。

参考：[A1 批次与 JD79665 驱动说明](https://oshwhub.com/chen7654/project_rarhcffq)。

## 独立对照：MoInk 的 C3 方案（不是第四个 S3 构建目标）

**MoInk 最值得借鉴的是离线传照片。** 手机直接连接设备自己的 Wi-Fi 热点，用浏览器打开设备提供的页面，选图、裁剪、转为四色，再传给设备刷屏。走这条本地流程，不需要路由器、外部服务器、App 或账号；照片只在手机和设备之间传输。MoInk 也支持可选联网，但联网不是本地传图的前提。

这和「墨水屏断电后还能显示上一次的画面」不同：MoInk 在没有互联网时仍能**换一张新照片**。本目录的三套 S3 配置适配的是 InkSight 屏幕驱动，未集成 MoInk 的热点传图页面；设备有配网热点，也不等于支持离线传照片。

**离线创作页能做什么：**

- **裁剪照片：** 页面内嵌 [Cropper.js](https://github.com/fengyuanchen/cropperjs)，手机连设备热点就能裁剪、缩放和旋转，无需联网加载脚本。
- **四色暖调：** `eink-warm` 针对黑、白、黄、红调色板做暖调映射，可配合自动色阶改善明暗对比。它用屏幕能显示的红黄表现冷调照片中的色彩差异，减少画面量化后只剩黑白灰的情况，呈现偏暖的胶片感。暖调映射负责选色，Atkinson 等抖动算法负责用像素排列表现过渡，两者作用不同。
- **便签排版：** 不上传照片也能写便签。在同一画布上添加多段文字，分别设置四色字、描边、底色、位置和旋转方向，预览后发送到屏幕。

**供电看硬件版本：** 按作品的外壳与供电方案，可选 Type-C 直插或配无线充电接收模块，壳内也预留锂电池空间。预留空间不等于板上带充电管理；无线供电和电池充电取决于实际安装的电源板，不是切换 S3 固件批次就能启用的功能。

[MakerWorld 作品 2998451](https://makerworld.com.cn/models/2998451) · [atunverse/moink 源码，核对版本 `343be94`](https://github.com/atunverse/moink/tree/343be94486388bbbe19891232e6bd79c0fce343c)。MoInk 使用 ESP32-C3 SuperMini（无 PSRAM）和 ESP-IDF；手机浏览器完成裁剪、四色量化与抖动（含 Atkinson），再把打包帧发给设备。它有自己的固件、帧协议和接线预设。

排查 A1 中缝时，注意区分**接收图像、写入窗口、控制器扫描尺寸**。MoInk 的 [A1 驱动](https://github.com/atunverse/moink/blob/343be94486388bbbe19891232e6bd79c0fce343c/src/epd_drv.c#L284-L318)接收 768×552，以 `(0,0)` 为起点顺序写 552 行，但控制器扫描 **768×600**；没有本项目立创新批次方案的 **800×600、X+32**。作者在[帧协议与实测记录](https://github.com/atunverse/moink/blob/343be94486388bbbe19891232e6bd79c0fce343c/docs/CONTRACT.md#L96-L141)中说明已移除旧的 800×600 诊断档。MoInk 的 `A1` 不能据此直接等同于本项目的 `A1_old` 或 `a1_new`。

接线也要按项目区分：例如 MoInk 的 C3 默认档用 **RST=GPIO3**，krstc 的 C3 用 **GPIO2**，本项目三套 S3 用 **GPIO8**。
