#include "API.h"

void API::DrawText(const char* text, int x, int y,
                   int fgcolor, int bgcolor,
                   uint8_t size,
                   bool nobg, bool centered) {
  lcd.setTextSize(size);
  if (nobg) {
    lcd.setTextColor(fgcolor);
  } else {
    lcd.setTextColor(fgcolor, bgcolor);
  }

  if (centered) {
    int textWidth  = lcd.textWidth(text);
    int textHeight = lcd.fontHeight();
    lcd.setCursor(((lcd.width()  - textWidth)  / 2) + x,
                  ((lcd.height() - textHeight) / 2) + y);
  } else {
    lcd.setCursor(x, y);
  }
  lcd.print(text);
}

bool API::PollTouch(Vector2& coord)
{
    int32_t x, y;

    if (lcd.getTouch(&x, &y)) {
        coord.x = x;
        coord.y = y;
        return true;
    }

    coord.x = -1;
    coord.y = -1;
    return false;
}

bool API::DrawRaw(const char* path, int x, int y, int w, int h)
{
    // Always use SD_MMC on this board
    File f = SD_MMC.open(path, FILE_READ);
    if (!f) {
        Serial.printf("Failed to open %s\n", path);
        return false;
    }

    uint16_t* lineBuf = (uint16_t*) heap_caps_malloc(w * 2, MALLOC_CAP_8BIT);
    if (!lineBuf) {
        Serial.println("Failed to allocate line buffer");
        f.close();
        return false;
    }

    for (int row = 0; row < h; row++) {
        size_t bytesRead = f.read((uint8_t*)lineBuf, w * 2);
        if (bytesRead != w * 2) {
            Serial.printf("Unexpected EOF in %s at row %d\n", path, row);
            f.close();
            free(lineBuf);
            return false;
        }

        lcd.pushImage(x, y + row, w, 1, lineBuf);
    }

    free(lineBuf);
    f.close();
    return true;
}

