#ifndef OLED_H_
#define OLED_H_

#include "ti_msp_dl_config.h"
#include <stdint.h>
#include <string.h>

// ─────────────────────────────────────────────────────────────────────────────
// Display dimensions
// ─────────────────────────────────────────────────────────────────────────────
#define OLED_W   128
#define OLED_H    64

// ─────────────────────────────────────────────────────────────────────────────
// Pin mapping — match the names you gave in SysConfig GPIO group "OLED_CTRL"
//   CS   → PB3   GPIO_OLED_CTRL_CS_PIN
//   DC   → PB6   GPIO_OLED_CTRL_DC_PIN
//   RST  → PB7   GPIO_OLED_CTRL_RST_PIN
//   Port → GPIOB (all three share the same port)
// ─────────────────────────────────────────────────────────────────────────────
#define OLED_PORT       OLED_CTRL_PORT

#define OLED_CS_PIN     OLED_CTRL_CS_PIN
#define OLED_DC_PIN     OLED_CTRL_DC_PIN
#define OLED_RST_PIN    OLED_CTRL_RST_PIN

// ─────────────────────────────────────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────────────────────────────────────
void OLED_Init(void);
void OLED_Clear(void);
void OLED_Fill(uint8_t color);
void OLED_Flush(void);                                          // push framebuf → display

void OLED_DrawPixel(int16_t x, int16_t y, uint8_t color);
void OLED_DrawHLine(int16_t x, int16_t y, int16_t len, uint8_t color);
void OLED_DrawVLine(int16_t x, int16_t y, int16_t len, uint8_t color);
void OLED_DrawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color);
void OLED_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color);
void OLED_DrawCircle(int16_t cx, int16_t cy, int16_t r, uint8_t color);
void OLED_DrawBitmap(int16_t x, int16_t y, int16_t w, int16_t h, const uint8_t *bmp);

// Text (uses built-in 5×7 font, scale ≥ 1)
void OLED_DrawChar(int16_t x, int16_t y, char c, uint8_t color, uint8_t scale);
void OLED_DrawString(int16_t x, int16_t y, const char *str, uint8_t color, uint8_t scale);

#endif // OLED_H_
