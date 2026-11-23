#ifndef INKSIGHT_DISPLAY_H
#define INKSIGHT_DISPLAY_H

#include <Arduino.h>

// Color constants for 4-color displays
#define COLOR_BLACK   0x00
#define COLOR_WHITE   0x01
#define COLOR_RED     0x02
#define COLOR_YELLOW  0x03

// Look up glyph data for a character (5x7 pixel font)
const uint8_t* getGlyph(char c);

// Draw scaled text into imgBuf at (x, y) - black only
void drawText(const char *msg, int x, int y, int scale);

// Draw scaled text with color (for 4-color displays)
void drawTextColor(const char *msg, int x, int y, int scale, uint8_t color);

// Show WiFi setup screen with AP name
void showSetupScreen(const char *apName);

// Show centered error message on screen
void showError(const char *msg);

// Show diagnostic screen with up to 4 lines
void showDiagnostic(const char *line1, const char *line2, const char *line3, const char *line4);

int currentPeriodIndex();

void updateTimeDisplay();

// Smart display: uses no-flash partial refresh normally, full refresh every N cycles
void smartDisplay(const uint8_t *image);

// Show mode name preview screen (displayed briefly on double-click before loading)
void showModePreview(const char *modeName);

#endif // INKSIGHT_DISPLAY_H
