// GxEPD2 driver for 3.8" 800x600 4-color e-paper, JD79665AA controller
// Based on GxEPD2_750c_GDEM075F52 by Jean-Marc Zingg
// Adapted for 800x600 BWRY panel

#ifndef _GxEPD2_380_JD79665_H_
#define _GxEPD2_380_JD79665_H_

#include "../GxEPD2_EPD.h"

class GxEPD2_380_JD79665 : public GxEPD2_EPD
{
  public:
    // attributes
#ifdef EPD_A1_NEW_BATCH
    static const uint16_t WIDTH = 800, HEIGHT = 600;
#else
    static const uint16_t WIDTH = 768, HEIGHT = 552;
#endif
    static const uint16_t WIDTH_VISIBLE = WIDTH;
    static const GxEPD2::Panel panel = GxEPD2::GDEM0380F52;
    static const bool hasColor = true;
    static const bool hasPartialUpdate = false;   // JD79665 4色无真正部分刷新
    static const bool hasFastPartialUpdate = false;
    static const bool useFastFullUpdate = false;
    static const uint16_t power_on_time = 200;      // ms
    static const uint16_t power_off_time = 150;     // ms
    static const uint16_t full_refresh_time = 30000;  // ms, 4色屏较慢
    static const uint16_t partial_refresh_time = 30000;

    // constructor
    GxEPD2_380_JD79665(int16_t cs, int16_t dc, int16_t rst, int16_t busy);

    // methods (virtual)
    void clearScreen(uint8_t value = 0xFF);
    void clearScreen(uint8_t black_value, uint8_t color_value);
    void writeScreenBuffer(uint8_t value = 0xFF);
    void writeScreenBuffer(uint8_t black_value, uint8_t color_value);

    // 1bpp 输入（黑白图）
    void writeImage(const uint8_t bitmap[], int16_t x, int16_t y, int16_t w, int16_t h,
                    bool invert = false, bool mirror_y = false, bool pgm = false);
    void writeImagePart(const uint8_t bitmap[], int16_t x_part, int16_t y_part,
                        int16_t w_bitmap, int16_t h_bitmap,
                        int16_t x, int16_t y, int16_t w, int16_t h,
                        bool invert = false, bool mirror_y = false, bool pgm = false);

    // 双平面输入（black + color/red），兼容 GxEPD2_4C 生态
    void writeImage(const uint8_t* black, const uint8_t* color,
                    int16_t x, int16_t y, int16_t w, int16_t h,
                    bool invert = false, bool mirror_y = false, bool pgm = false);
    void writeImagePart(const uint8_t* black, const uint8_t* color,
                        int16_t x_part, int16_t y_part,
                        int16_t w_bitmap, int16_t h_bitmap,
                        int16_t x, int16_t y, int16_t w, int16_t h,
                        bool invert = false, bool mirror_y = false, bool pgm = false);

    // ★ 新增：直接写 4 色 2bpp 数据（你 main.cpp 用的就是这个）
    // 编码：每字节 4 像素，MSB 先出，每像素 2bit
    //   0x00=黑, 0x01=白, 0x02=黄, 0x03=红
    void writeImage2bpp(const uint8_t* buf2bpp, int16_t x, int16_t y, int16_t w, int16_t h);

    // native 4-bit sprite 输入（照搬 GDEM075F52）
    void writeNative(const uint8_t* data1, const uint8_t* data2,
                     int16_t x, int16_t y, int16_t w, int16_t h,
                     bool invert = false, bool mirror_y = false, bool pgm = false);

    void refresh(bool partial_update_mode = false);
    void refresh(int16_t x, int16_t y, int16_t w, int16_t h);
    void powerOff();
    void hibernate();

  private:
    void _refresh(bool partial_update_mode);
    void _setPartialRamArea(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool partial_mode = true);
    void _PowerOn();
    void _PowerOff();
    void _InitDisplay();
    void _waitWhileBusyQuick(const char* comment, uint32_t timeout_ms);
};

#endif
