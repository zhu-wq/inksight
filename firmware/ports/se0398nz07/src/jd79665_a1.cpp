#include "epd_driver.h"
#include "config.h"
#include <epd4c/GxEPD2_380_JD79665.h>

static GxEPD2_380_JD79665 panel(PIN_EPD_CS, PIN_EPD_DC, PIN_EPD_RST, PIN_EPD_BUSY);
static constexpr int offsetX = GxEPD2_380_JD79665::WIDTH - W;
static_assert(W == 768 && H == 552 && EPD_BPP == 2, "Expected the 3.98-inch BWRY canvas");

void gpioInit() {
    pinMode(PIN_CFG_BTN, INPUT_PULLUP);
    SPI.begin(PIN_EPD_SCK, -1, PIN_EPD_MOSI, PIN_EPD_CS);
    panel.selectSPI(SPI, SPISettings(4000000, MSBFIRST, SPI_MODE0));
    panel.init(0);
}

// The ZIP driver runs _InitDisplay lazily on the first write.
void epdInit() {}
void epdInitFast() {}

static void finishFrame() {
    panel.refresh(false);
    panel.powerOff();
}

void epdDisplay2bpp(const uint8_t *image) {
    if (!image) return;
    panel.writeImage2bpp(image, offsetX, 0, W, H);
    finishFrame();
}

void epdDisplay(const uint8_t *image) {
    if (!image) return;
    panel.writeImage(image, offsetX, 0, W, H);
    finishFrame();
}

void epdDisplayFast(const uint8_t *image) { epdDisplay(image); }
void epdPartialDisplay(uint8_t *, int, int, int, int) { epdDisplay(imgBuf); }
void epdSleep() { panel.hibernate(); }
