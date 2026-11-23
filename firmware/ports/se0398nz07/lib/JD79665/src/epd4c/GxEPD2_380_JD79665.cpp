// GxEPD2 driver for 3.8" 800x600 4-color e-paper, JD79665AA controller
// Based on GxEPD2_750c_GDEM075F52, init sequence from original epd_driver.cpp

#include "GxEPD2_380_JD79665.h"

// ---------- 构造 ----------
GxEPD2_380_JD79665::GxEPD2_380_JD79665(int16_t cs, int16_t dc, int16_t rst, int16_t busy) :
  GxEPD2_EPD(cs, dc, rst, busy, LOW, 4000000, WIDTH, HEIGHT, panel,
             hasColor, hasPartialUpdate, hasFastPartialUpdate)
{
}

// ---------- clear ----------
void GxEPD2_380_JD79665::clearScreen(uint8_t value)
{
  clearScreen(value, 0xFF);
}

void GxEPD2_380_JD79665::clearScreen(uint8_t black_value, uint8_t color_value)
{
  writeScreenBuffer(black_value, color_value);
  refresh();
}

void GxEPD2_380_JD79665::writeScreenBuffer(uint8_t value)
{
  writeScreenBuffer(value, 0xFF);
}

void GxEPD2_380_JD79665::writeScreenBuffer(uint8_t black_value, uint8_t color_value)
{
  if (!_init_display_done) _InitDisplay();
  _writeCommand(0x10);
  _startTransfer();
  // 全屏 2bpp 填充：白=0x55(01b×4)，黑=0x00
  uint8_t fill = (0xFF == black_value) ? 0x55 : 0x00;
  for (uint32_t i = 0; i < uint32_t(WIDTH) * uint32_t(HEIGHT) / 4; i++)
  {
    _transfer(fill);
  }
  _endTransfer();
  _initial_write = false;
}

// ---------- writeImage: 1bpp 黑白 ----------
void GxEPD2_380_JD79665::writeImage(const uint8_t bitmap[], int16_t x, int16_t y,
                                    int16_t w, int16_t h,
                                    bool invert, bool mirror_y, bool pgm)
{
  if (!_init_display_done) _InitDisplay();
  if (_initial_write) writeScreenBuffer();

  int16_t wb = (w + 7) / 8;
  x -= x % 8;
  w = wb * 8;
  if ((w <= 0) || (h <= 0)) return;
  if (x >= int16_t(WIDTH) || y >= int16_t(HEIGHT)) return;

  _writeCommand(0x10);
  _startTransfer();
  for (int16_t i = 0; i < int16_t(HEIGHT); i++)
  {
    for (int16_t j = 0; j < int16_t(WIDTH); j += 8)
    {
      uint8_t data = 0xFF;
      if ((j >= x) && (j < x + w) && (i >= y) && (i < y + h))
      {
        uint32_t idx = mirror_y
          ? (j - x) / 8 + uint32_t((h - 1 - (i - y))) * wb
          : (j - x) / 8 + uint32_t(i - y) * wb;
        if (pgm)
        {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
          data = pgm_read_byte(&bitmap[idx]);
#else
          data = bitmap[idx];
#endif
        }
        else
        {
          data = bitmap[idx];
        }
        if (invert) data = ~data;
      }
      // 1bpp → 2bpp：黑像素 00b，白像素 01b
      for (int16_t k = 0; k < 2; k++)
      {
        uint8_t out = 0;
        for (int16_t l = 0; l < 4; l++)
        {
          out <<= 2;
          out |= (data & 0x80) ? 0x01 : 0x00;
          data <<= 1;
        }
        _transfer(out);
      }
    }
  }
  _endTransfer();
}

// ---------- writeImagePart: 1bpp ----------
void GxEPD2_380_JD79665::writeImagePart(const uint8_t bitmap[],
                                        int16_t x_part, int16_t y_part,
                                        int16_t w_bitmap, int16_t h_bitmap,
                                        int16_t x, int16_t y, int16_t w, int16_t h,
                                        bool invert, bool mirror_y, bool pgm)
{
  if (!_init_display_done) _InitDisplay();
  if (_initial_write) writeScreenBuffer();
  if ((w_bitmap < 0) || (h_bitmap < 0) || (w < 0) || (h < 0)) return;
  if ((x_part < 0) || (x_part >= w_bitmap)) return;
  if ((y_part < 0) || (y_part >= h_bitmap)) return;

  int16_t wb_bitmap = (w_bitmap + 7) / 8;
  x_part -= x_part % 8;
  w = w_bitmap - x_part < w ? w_bitmap - x_part : w;
  h = h_bitmap - y_part < h ? h_bitmap - y_part : h;
  w += x % 8;
  x -= x % 8;
  w = 8 * ((w + 7) / 8);

  int16_t x1 = x < 0 ? 0 : x;
  int16_t y1 = y < 0 ? 0 : y;
  int16_t w1 = x + w < int16_t(WIDTH) ? w : int16_t(WIDTH) - x;
  int16_t h1 = y + h < int16_t(HEIGHT) ? h : int16_t(HEIGHT) - y;
  int16_t dx = x1 - x;
  int16_t dy = y1 - y;
  w1 -= dx;
  h1 -= dy;
  if ((w1 <= 0) || (h1 <= 0)) return;

  _writeCommand(0x10);
  _startTransfer();
  for (int16_t i = 0; i < int16_t(HEIGHT); i++)
  {
    for (int16_t j = 0; j < int16_t(WIDTH); j += 8)
    {
      uint8_t data = 0xFF;
      if ((j >= x1) && (j < x1 + w1) && (i >= y1) && (i < y1 + h1))
      {
        uint32_t idx = mirror_y
          ? (x_part + j - x1) / 8 + uint32_t((h_bitmap - 1 - (y_part + i - y1))) * wb_bitmap
          : (x_part + j - x1) / 8 + uint32_t(y_part + i - y1) * wb_bitmap;
        if (pgm)
        {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
          data = pgm_read_byte(&bitmap[idx]);
#else
          data = bitmap[idx];
#endif
        }
        else
        {
          data = bitmap[idx];
        }
        if (invert) data = ~data;
      }
      for (int16_t k = 0; k < 2; k++)
      {
        uint8_t out = 0;
        for (int16_t l = 0; l < 4; l++)
        {
          out <<= 2;
          out |= (data & 0x80) ? 0x01 : 0x00;
          data <<= 1;
        }
        _transfer(out);
      }
    }
  }
  _endTransfer();
}

// ---------- writeImage: black + color(red) 双平面 ----------
// 编码：color=1 表示红/黄区域（color 位），black=0 表示黑
// 输出 2bpp：黑=00, 白=01, 黄=10（暂不外用）, 红=11
void GxEPD2_380_JD79665::writeImage(const uint8_t* black, const uint8_t* color,
                                    int16_t x, int16_t y, int16_t w, int16_t h,
                                    bool invert, bool mirror_y, bool pgm)
{
  if (!black && !color) return;
  if (!color) return writeImage(black, x, y, w, h, invert, mirror_y, pgm);
  if (!_init_display_done) _InitDisplay();
  if (_initial_write) writeScreenBuffer();

  int16_t wb = (w + 7) / 8;
  x -= x % 8;
  w = wb * 8;
  if ((w <= 0) || (h <= 0)) return;

  _writeCommand(0x10);
  _startTransfer();
  for (int16_t i = 0; i < int16_t(HEIGHT); i++)
  {
    for (int16_t j = 0; j < int16_t(WIDTH); j += 8)
    {
      uint8_t black_data = 0xFF, color_data = 0xFF;
      if ((j >= x) && (j < x + w) && (i >= y) && (i < y + h))
      {
        uint32_t idx = mirror_y
          ? (j - x) / 8 + uint32_t((h - 1 - (i - y))) * wb
          : (j - x) / 8 + uint32_t(i - y) * wb;
        if (pgm)
        {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
          black_data = pgm_read_byte(&black[idx]);
          color_data = pgm_read_byte(&color[idx]);
#else
          black_data = black[idx];
          color_data = color[idx];
#endif
        }
        else
        {
          black_data = black[idx];
          color_data = color[idx];
        }
        if (invert) { black_data = ~black_data; color_data = ~color_data; }
      }
      for (int16_t k = 0; k < 2; k++)
      {
        uint8_t out = 0;
        for (int16_t l = 0; l < 4; l++)
        {
          out <<= 2;
          bool is_black = !(black_data & 0x80);
          bool is_color = !(color_data & 0x80);
          if (is_black)      out |= 0x00;  // 黑
          else if (is_color) out |= 0x03;  // 红
          else               out |= 0x01;  // 白
          black_data <<= 1;
          color_data <<= 1;
        }
        _transfer(out);
      }
    }
  }
  _endTransfer();
}

// ---------- writeImagePart: black + color 双平面 ----------
void GxEPD2_380_JD79665::writeImagePart(const uint8_t* black, const uint8_t* color,
                                        int16_t x_part, int16_t y_part,
                                        int16_t w_bitmap, int16_t h_bitmap,
                                        int16_t x, int16_t y, int16_t w, int16_t h,
                                        bool invert, bool mirror_y, bool pgm)
{
  if (!black && !color) return;
  if (!color) return writeImagePart(black, x_part, y_part, w_bitmap, h_bitmap,
                                    x, y, w, h, invert, mirror_y, pgm);
  if (!_init_display_done) _InitDisplay();
  if (_initial_write) writeScreenBuffer();

  if ((w_bitmap < 0) || (h_bitmap < 0) || (w < 0) || (h < 0)) return;
  if ((x_part < 0) || (x_part >= w_bitmap)) return;
  if ((y_part < 0) || (y_part >= h_bitmap)) return;

  int16_t wb_bitmap = (w_bitmap + 7) / 8;
  x_part -= x_part % 8;
  w = w_bitmap - x_part < w ? w_bitmap - x_part : w;
  h = h_bitmap - y_part < h ? h_bitmap - y_part : h;
  w += x % 8;
  x -= x % 8;
  w = 8 * ((w + 7) / 8);

  int16_t x1 = x < 0 ? 0 : x;
  int16_t y1 = y < 0 ? 0 : y;
  int16_t w1 = x + w < int16_t(WIDTH) ? w : int16_t(WIDTH) - x;
  int16_t h1 = y + h < int16_t(HEIGHT) ? h : int16_t(HEIGHT) - y;
  int16_t dx = x1 - x;
  int16_t dy = y1 - y;
  w1 -= dx;
  h1 -= dy;
  if ((w1 <= 0) || (h1 <= 0)) return;

  _writeCommand(0x10);
  _startTransfer();
  for (int16_t i = 0; i < int16_t(HEIGHT); i++)
  {
    for (int16_t j = 0; j < int16_t(WIDTH); j += 8)
    {
      uint8_t black_data = 0xFF, color_data = 0xFF;
      if ((j >= x1) && (j < x1 + w1) && (i >= y1) && (i < y1 + h1))
      {
        uint32_t idx = mirror_y
          ? (x_part + j - x1) / 8 + uint32_t((h_bitmap - 1 - (y_part + i - y1))) * wb_bitmap
          : (x_part + j - x1) / 8 + uint32_t(y_part + i - y1) * wb_bitmap;
        if (pgm)
        {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
          black_data = pgm_read_byte(&black[idx]);
          color_data = pgm_read_byte(&color[idx]);
#else
          black_data = black[idx];
          color_data = color[idx];
#endif
        }
        else
        {
          black_data = black[idx];
          color_data = color[idx];
        }
        if (invert) { black_data = ~black_data; color_data = ~color_data; }
      }
      for (int16_t k = 0; k < 2; k++)
      {
        uint8_t out = 0;
        for (int16_t l = 0; l < 4; l++)
        {
          out <<= 2;
          bool is_black = !(black_data & 0x80);
          bool is_color = !(color_data & 0x80);
          if (is_black)      out |= 0x00;
          else if (is_color) out |= 0x03;
          else               out |= 0x01;
          black_data <<= 1;
          color_data <<= 1;
        }
        _transfer(out);
      }
    }
  }
  _endTransfer();
}

// ---------- writeImage2bpp: 直接写 4 色 2bpp ----------
// 数据布局：每字节 4 像素，MSB 先出，每像素 2bit
// 编码：0x00=黑, 0x01=白, 0x02=黄, 0x03=红
// 起始 (x,y) 需 4 像素对齐（x%4==0）；w/h 任意，但内部按 4 像素/字节处理
void GxEPD2_380_JD79665::writeImage2bpp(const uint8_t* buf2bpp,
                                        int16_t x, int16_t y, int16_t w, int16_t h)
{
  if (!buf2bpp) return;
  if (!_init_display_done) _InitDisplay();
  if (_initial_write) writeScreenBuffer();

  // 边界裁剪
  int16_t x1 = x < 0 ? 0 : x;
  int16_t y1 = y < 0 ? 0 : y;
  int16_t w1 = x + w < int16_t(WIDTH) ? w : int16_t(WIDTH) - x;
  int16_t h1 = y + h < int16_t(HEIGHT) ? h : int16_t(HEIGHT) - y;
  if ((w1 <= 0) || (h1 <= 0)) return;

  // 仅支持整屏写入（简化版，和你 main.cpp 用法一致）
  // 如需局部，可自行按 _setPartialRamArea 扩展
  if ((x1 != 0) || (y1 != 0) || (w1 != int16_t(WIDTH)) || (h1 != int16_t(HEIGHT)))
  {
    // 局部写：逐行取，背景用白填充
    _writeCommand(0x10);
    _startTransfer();
    int16_t srcRowBytes = (w + 3) / 4;  // 每行 2bpp 字节数
    for (int16_t i = 0; i < int16_t(HEIGHT); i++)
    {
      for (int16_t j = 0; j < int16_t(WIDTH); j += 4)
      {
        uint8_t out = 0x55;  // 默认白
        if ((j >= x1) && (j < x1 + w1) && (i >= y1) && (i < y1 + h1))
        {
          int16_t srcJ = (j - x1) / 4;
          int16_t srcI = i - y1;
          out = buf2bpp[srcI * srcRowBytes + srcJ];
        }
        _transfer(out);
      }
    }
    _endTransfer();
    return;
  }

  // 整屏：直接搬
  _writeCommand(0x10);
  _startTransfer();
  uint32_t total = uint32_t(WIDTH) * uint32_t(HEIGHT) / 4;
  for (uint32_t i = 0; i < total; i++)
  {
    _transfer(buf2bpp[i]);
  }
  _endTransfer();
}

// ---------- writeNative（照搬 GDEM075F52，未改）----------
void GxEPD2_380_JD79665::writeNative(const uint8_t* data1, const uint8_t* data2,
                                     int16_t x, int16_t y, int16_t w, int16_t h,
                                     bool invert, bool mirror_y, bool pgm)
{
  if (!data1) return;
  if (!_init_display_done) _InitDisplay();
  if (_initial_write) writeScreenBuffer();
  (void)data2; (void)invert; (void)mirror_y; (void)pgm;

  int16_t wb = (w + 3) / 4;
  x -= x % 4;
  w = wb * 4;
  if ((w <= 0) || (h <= 0)) return;

  _writeCommand(0x10);
  _startTransfer();
  for (int16_t i = 0; i < int16_t(HEIGHT); i++)
  {
    for (int16_t j = 0; j < int16_t(WIDTH); j += 4)
    {
      uint8_t data = 0x55;
      if ((j >= x) && (j < x + w) && (i >= y) && (i < y + h))
      {
        uint32_t idx = (j - x) / 4 + uint32_t(i - y) * wb;
        if (pgm)
        {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
          data = pgm_read_byte(&data1[idx]);
#else
          data = data1[idx];
#endif
        }
        else
        {
          data = data1[idx];
        }
      }
      _transfer(data);
    }
  }
  _endTransfer();
}

// ---------- refresh ----------
void GxEPD2_380_JD79665::refresh(bool partial_update_mode)
{
  (void)partial_update_mode;
  _setPartialRamArea(0, 0, WIDTH, HEIGHT, false);
  _refresh(false);
}

void GxEPD2_380_JD79665::refresh(int16_t x, int16_t y, int16_t w, int16_t h)
{
  // 4色屏无真正部分刷新：落到全刷
  (void)x; (void)y; (void)w; (void)h;
  refresh(false);
}

void GxEPD2_380_JD79665::powerOff()
{
  _PowerOff();
}

void GxEPD2_380_JD79665::hibernate()
{
  _PowerOff();
  if (_rst >= 0)
  {
    _writeCommand(0x07);
    _writeData(0xA5);
    _hibernating = true;
    _init_display_done = false;
  }
}

// ---------- private ----------
void GxEPD2_380_JD79665::_refresh(bool partial_update_mode)
{
  (void)partial_update_mode;
  _writeCommand(0x12);
  _writeData(0x00);
  delay(1);
  _waitWhileBusyQuick("_refresh", full_refresh_time);
  _init_display_done = false;
}

void GxEPD2_380_JD79665::_setPartialRamArea(uint16_t x, uint16_t y,
                                            uint16_t w, uint16_t h,
                                            bool partial_mode)
{
  _writeCommand(0x83);
  _writeData(x / 256);
  _writeData(x % 256);
  _writeData((x + w - 1) / 256);
  _writeData((x + w - 1) % 256);
  _writeData(y / 256);
  _writeData(y % 256);
  _writeData((y + h - 1) / 256);
  _writeData((y + h - 1) % 256);
  _writeData(partial_mode ? 0x01 : 0x00);
}

void GxEPD2_380_JD79665::_PowerOn()
{
  if (!_power_is_on)
  {
    _writeCommand(0x04);
    _waitWhileBusyQuick("_PowerOn", power_on_time);
  }
  _power_is_on = true;
}

void GxEPD2_380_JD79665::_PowerOff()
{
  if (_power_is_on)
  {
    _writeCommand(0x02);
    _writeData(0x00);
    _waitWhileBusyQuick("_PowerOff", power_off_time);
  }
  _power_is_on = false;
}

// ★ 核心：完整照搬你 epd_driver.cpp 里 EPD_PANEL_38_JD79665_BWRY 的初始化序列
void GxEPD2_380_JD79665::_InitDisplay()
{
  // 复位
  if ((_rst >= 0) && (_hibernating || _initial_write))
  {
    digitalWrite(_rst, HIGH); delay(20);
    digitalWrite(_rst, LOW);  delay(2);
    digitalWrite(_rst, HIGH); delay(20);
    _waitWhileBusyQuick("_InitDisplay reset", 2000);
    _hibernating = false;
    _power_is_on = false;
  }

  // --- 你的初始化序列 ---
  _writeCommand(0xAA);
  _writeData(0x49); _writeData(0x55); _writeData(0x20);
  _writeData(0x08); _writeData(0x09); _writeData(0x18);

  _writeCommand(0x01);
  _writeData(0x3F);

  _writeCommand(0x00);
  _writeData(0x4B); _writeData(0x69);

  _writeCommand(0x05);
  _writeData(0x40); _writeData(0x1F); _writeData(0x1F); _writeData(0x2C);

  _writeCommand(0x08);
  _writeData(0x6F); _writeData(0x1F); _writeData(0x1F); _writeData(0x22);

  _writeCommand(0x06);
  _writeData(0x6F); _writeData(0x1F); _writeData(0x14); _writeData(0x14);

  _writeCommand(0x03);
  _writeData(0x00); _writeData(0x54); _writeData(0x00); _writeData(0x44);

  _writeCommand(0x60);
  _writeData(0x02); _writeData(0x00);

  _writeCommand(0x30);
  _writeData(0x08);

  _writeCommand(0x50);
  _writeData(0x3F);

  // 分辨率 800x600
  _writeCommand(0x61);
  _writeData(WIDTH / 256);
  _writeData(WIDTH % 256);
  _writeData(HEIGHT / 256);
  _writeData(HEIGHT % 256);

  _writeCommand(0x65);
  _writeData(0x10); _writeData(0x00); _writeData(0x20); _writeData(0x00);

  _writeCommand(0xE3);
  _writeData(0x2F);

  _writeCommand(0x84);
  _writeData(0x01);

  // 全窗口
  _setPartialRamArea(0, 0, WIDTH, HEIGHT, false);

  _writeCommand(0x04);
  _waitWhileBusyQuick("_InitDisplay (0x04)", 5000);
  _power_is_on = true; // 0x04 was issued above; allow powerOff() to send 0x02.

  _init_display_done = true;
}

// 带喂狗的 busy 等待
void GxEPD2_380_JD79665::_waitWhileBusyQuick(const char* comment, uint32_t timeout_ms)
{
  uint32_t start = millis();
  while (digitalRead(_busy) == _busy_level)  // LOW=忙
  {
    delay(10);
#if defined(ESP32)
    yield();  // 喂狗
#endif
    if (millis() - start > timeout_ms)
    {
      if (comment) Serial.printf("Busy Timeout: %s\n", comment);
      break;
    }
  }
}
