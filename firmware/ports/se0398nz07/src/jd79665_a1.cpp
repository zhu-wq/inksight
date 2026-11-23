#include "epd_driver.h"
#include "config.h"

// JD79665 path from krstc/InkSight_adapt_HUAWEI_eink @ 78ceea6.
// W/H remain the visible image size. The newer A1 has a larger controller RAM.
#ifdef EPD_A1_NEW_BATCH
static constexpr int ramW = 800, ramH = 600, offsetX = 32;
#else
static constexpr int ramW = W, ramH = H, offsetX = 0;
#endif
static_assert(W == 768 && H == 552 && EPD_BPP == 2, "Expected SE0398NZ07 A1");


// ── Software SPI (bit-bang) for 4.2" panels ──
// Avoids Busy Timeout on ESP32-C3 with non-default pins; no GxEPD2 dependency.

static void spiWriteByte(uint8_t data) {
    for (int i = 0; i < 8; i++) {
        digitalWrite(PIN_EPD_MOSI, (data & 0x80) ? HIGH : LOW);
        data <<= 1;
        digitalWrite(PIN_EPD_SCK, HIGH);
        digitalWrite(PIN_EPD_SCK, LOW);
    }
}

static void epdSendCommand(uint8_t cmd) {
    digitalWrite(PIN_EPD_DC, LOW);   // DC low = command
    digitalWrite(PIN_EPD_CS, LOW);
    spiWriteByte(cmd);
    digitalWrite(PIN_EPD_CS, HIGH);
}

static void epdSendData(uint8_t data) {
    digitalWrite(PIN_EPD_DC, HIGH);  // DC high = data
    digitalWrite(PIN_EPD_CS, LOW);
    spiWriteByte(data);
    digitalWrite(PIN_EPD_CS, HIGH);
}

static bool epdWaitBusy(unsigned long maxMs = 0) {
    unsigned long t0 = millis();
    unsigned long timeoutMs = maxMs > 0 ? maxMs :
        180000;
    while (digitalRead(PIN_EPD_BUSY) == LOW) {
        delay(10);
        if (millis() - t0 > timeoutMs) {
            Serial.println("EPD busy TIMEOUT!");
            return false;
        }
    }
    return true;
}

static void epdReset() {
    digitalWrite(PIN_EPD_RST, HIGH); delay(20);
    digitalWrite(PIN_EPD_RST, LOW);  delay(2);
    digitalWrite(PIN_EPD_RST, HIGH); delay(20);
}

// ── Helper: configure RAM window for full screen ────────────

static void epdSetFullWindow() {
    epdSendCommand(0x11);  // Data Entry Mode Setting
    epdSendData(0x03);     //   X increment, Y increment

    epdSendCommand(0x44);  // Set RAM X address range
    epdSendData(0x00);
    epdSendData((W - 1) / 8);

    epdSendCommand(0x45);  // Set RAM Y address range
    epdSendData(0x00);
    epdSendData(0x00);
    epdSendData((H - 1) & 0xFF);
    epdSendData(((H - 1) >> 8) & 0xFF);

    epdSendCommand(0x4E);  // Set RAM X address counter
    epdSendData(0x00);

    epdSendCommand(0x4F);  // Set RAM Y address counter
    epdSendData(0x00);
    epdSendData(0x00);
}

static void epdSendResolution() {
    epdSendCommand(0x61);
    epdSendData(ramW >> 8);
    epdSendData(ramW & 0xFF);
    epdSendData(ramH >> 8);
    epdSendData(ramH & 0xFF);
}

static void epdSetJd796xxFullWindow() {
    uint16_t x0 = 0;
    uint16_t x1 = ramW - 1;
    uint16_t y0 = 0;
    uint16_t y1 = ramH - 1;

    epdSendCommand(0x83);
    epdSendData((x0 >> 8) & 0xFF);
    epdSendData(x0 & 0xFF);
    epdSendData((x1 >> 8) & 0xFF);
    epdSendData(x1 & 0xFF);
    epdSendData((y0 >> 8) & 0xFF);
    epdSendData(y0 & 0xFF);
    epdSendData((y1 >> 8) & 0xFF);
    epdSendData(y1 & 0xFF);
    epdSendData(0x01);
}

// ── GPIO initialization ─────────────────────────────────────

void gpioInit() {
    pinMode(PIN_EPD_BUSY, INPUT);
    pinMode(PIN_EPD_RST,  OUTPUT);
    pinMode(PIN_EPD_DC,   OUTPUT);
    pinMode(PIN_EPD_CS,   OUTPUT);
    pinMode(PIN_EPD_SCK,  OUTPUT);
    pinMode(PIN_EPD_MOSI, OUTPUT);
    pinMode(PIN_CFG_BTN,  INPUT_PULLUP);
    digitalWrite(PIN_EPD_RST, HIGH);
    digitalWrite(PIN_EPD_CS,  HIGH);
    digitalWrite(PIN_EPD_SCK, LOW);
}

// ── EPD full init (standard mode) ──

void epdInit() {
    epdReset();
    Serial.printf("[EPD-init] JD79665 RAM %dx%d BUSY=%d\n", ramW, ramH, digitalRead(PIN_EPD_BUSY));
    epdWaitBusy();
    delay(30);

    epdSendCommand(0xAA);
    epdSendData(0x49);
    epdSendData(0x55);
    epdSendData(0x20);
    epdSendData(0x08);
    epdSendData(0x09);
    epdSendData(0x18);

    epdSendCommand(0x01);
    epdSendData(0x3F);

    epdSendCommand(0x00);
    epdSendData(0x4B);
    epdSendData(0x69);

    epdSendCommand(0x05);
    epdSendData(0x40);
    epdSendData(0x1F);
    epdSendData(0x1F);
    epdSendData(0x2C);

    epdSendCommand(0x08);
    epdSendData(0x6F);
    epdSendData(0x1F);
    epdSendData(0x1F);
    epdSendData(0x22);

    epdSendCommand(0x06);
    epdSendData(0x6F);
    epdSendData(0x1F);
    epdSendData(0x14);
    epdSendData(0x14);

    epdSendCommand(0x03);
    epdSendData(0x00);
    epdSendData(0x54);
    epdSendData(0x00);
    epdSendData(0x44);

    epdSendCommand(0x60);
    epdSendData(0x02);
    epdSendData(0x00);

    epdSendCommand(0x30);
    epdSendData(0x08);

    epdSendCommand(0x50);
    epdSendData(0x3F);

    epdSendResolution();

    epdSendCommand(0x65);
    epdSendData(0x10);
    epdSendData(0x00);
    epdSendData(0x20);
    epdSendData(0x00);

    epdSendCommand(0xE3);
    epdSendData(0x2F);

    epdSendCommand(0x84);
    epdSendData(0x01);

    epdSetJd796xxFullWindow();
    epdSendCommand(0x04);
    epdWaitBusy();
}

// ── EPD fast init (loads fast-refresh LUT via temperature register) ──
// Based on official Waveshare epd4in2_V2 Init_Fast() implementation.
// The 0x1A register sets a temperature value that selects faster internal LUT.
// 0x6E = ~1.5s refresh, 0x5A = ~1s refresh.

void epdInitFast() {
    epdInit();
}

// ── EPD full-screen display (standard full refresh, 0xF7) ───
// Clears all ghosting but has visible black-white flash (~3-4s).

static uint8_t packMonoPixelGroup(const uint8_t *image, int rowBytes, int y, int x) {
    uint8_t packed = 0;
    for (int bit = 0; bit < 4; bit++) {
        int px = x + bit;
        bool isBlack = (image[y * rowBytes + px / 8] & (0x80 >> (px % 8))) == 0;
        uint8_t color = isBlack ? 0x00 : 0x01;
        packed |= color << (6 - bit * 2);
    }
    return packed;
}

static uint8_t epdRemap2bppColor(uint8_t color) {
    return color & 0x03;
}

static void epdWriteMapped2bpp(const uint8_t *buf2bpp) {
    epdSendCommand(0x10);
#ifdef EPD_A1_NEW_BATCH
    for (int y = 0; y < ramH; ++y) {
        for (int x = 0; x < ramW; x += 4) {
            epdSendData(y < H && x >= offsetX && x < offsetX + W
                ? buf2bpp[y * (W / 4) + (x - offsetX) / 4] : 0x55);
        }
    }
#else
    for (int i = 0; i < COLOR_BUF_LEN; i++) {
        uint8_t src = buf2bpp[i];
        uint8_t dst = 0;
        dst |= epdRemap2bppColor((src >> 6) & 0x03) << 6;
        dst |= epdRemap2bppColor((src >> 4) & 0x03) << 4;
        dst |= epdRemap2bppColor((src >> 2) & 0x03) << 2;
        dst |= epdRemap2bppColor(src & 0x03);
        epdSendData(dst);
    }
#endif
}

static void epdPowerOff() {
    epdSendCommand(0x02);
    epdSendData(0x00);
    epdWaitBusy();
}

static void epdJd796xxRefresh() {
    epdSetJd796xxFullWindow();
    epdSendCommand(0x12);
    epdSendData(0x00);

    unsigned long t0 = millis();
    bool busyOk = epdWaitBusy(180000);
    unsigned long elapsed = millis() - t0;
    if (!busyOk) {
        Serial.printf("[EPD] JD79665 refresh BUSY timeout after %lums\n", elapsed);
    } else if (elapsed < 250) {
        Serial.printf("[EPD] JD79665 refresh BUSY only %lums, fallback wait\n", elapsed);
        delay(14000);
    } else {
        delay(500);
    }

    epdPowerOff();
}

static void epdSend2bppAndRefresh(const uint8_t *buf2bpp) {
    for (int attempt = 0; attempt < 3; attempt++) {
        unsigned long t0 = millis();
        Serial.printf("[EPD] attempt %d start BUSY=%d\n", attempt, digitalRead(PIN_EPD_BUSY));
        epdInit();
        Serial.printf("[EPD] init done %lums BUSY=%d\n", millis()-t0, digitalRead(PIN_EPD_BUSY));
        epdWriteMapped2bpp(buf2bpp);
        Serial.printf("[EPD] data done %lums\n", millis()-t0);
        epdJd796xxRefresh();
        Serial.printf("[EPD] all done %lums\n", millis()-t0);
        return;
    }
    Serial.println("[EPD] display failed after 3 attempts");
}

void epdDisplay(const uint8_t *image) {
    if (!colorBuf) { Serial.println("[EPD] colorBuf alloc failed"); return; }
    int rowBytes = W / 8;
    int out = 0;
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x += 4) {
            colorBuf[out++] = packMonoPixelGroup(image, rowBytes, y, x);
        }
    }
    epdSend2bppAndRefresh(colorBuf);
}

void epdDisplay2bpp(const uint8_t *image2bpp) {
    epdSend2bppAndRefresh(image2bpp);
}

// ── EPD deep clear (multi-cycle anti-ghosting) ──────────────
// SSD1683 BW: cycles all-black/all-white via 0x24/0x26 registers.
// 4-color panels: falls back to epdDisplay (no register-level deep clear).

void epdDisplayDeepClear(const uint8_t *image) {
    epdDisplay(image);
}

// ── EPD full-screen display (fast refresh, 0xC7) ────────────

void epdDisplayFast(const uint8_t *image) {
    epdDisplay(image);
}

// ── EPD partial refresh ─────────────────────────────────────

void epdPartialDisplay(uint8_t *data, int xStart, int yStart, int xEnd, int yEnd) {
    // JD79665 has no partial refresh path in krstc.
    epdDisplay(imgBuf);
}

bool epdSupportsPartialRefresh() {
    return false;
}

void epdPartialDisplayWithOld(uint8_t *data, const uint8_t *oldData, int xStart, int yStart, int xEnd, int yEnd) {
    (void)data;
    (void)oldData;
    (void)xStart;
    (void)yStart;
    (void)xEnd;
    (void)yEnd;
    epdDisplay(imgBuf);
}

// ── EPD sleep ───────────────────────────────────────────────

void epdSleep() {
    epdSendCommand(0x07);
    epdSendData(0xA5);
    delay(200);
}
