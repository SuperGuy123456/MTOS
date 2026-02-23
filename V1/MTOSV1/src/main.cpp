#include <Arduino.h>
#include "LGFX_config.h"
#include "Button.h"
#include "Wasm.h"
#include <functional>
#include "API.h"

#include <Wire.h>

#include "FS.h"
#include "SD_MMC.h"

// screen dimensions are 320x480 (panel), your UI uses 320x240 region
enum State
{
  SPLASH,
  MENU,
  BOOT,
  RUN
};

SimpleButton StartBtn("start", 190, 215, TFT_WHITE, TFT_RED, 3);
InvisButton install (17, 0, 64, 64);
InvisButton runBtn  (91, 0, 64, 64);
InvisButton sdv     (165, 0, 64, 64);
InvisButton sett    (239, 0, 64, 64);

bool update = false;
State cstate;

TCA9554 tca(0x20);

Wasm wasm;

void reset_lcd_via_tca()
{
  tca.pinMode1(1, OUTPUT);
  // match factory LVGL demo
  tca.write1(1, 1);
  delay(10);
  tca.write1(1, 0);
  delay(10);
  tca.write1(1, 1);
  delay(120);
}

void printHomeDir() {
  Serial.println("Listing / (SD root):");

  File root = SD_MMC.open("/");
  if (!root) {
    Serial.println("Failed to open root directory!");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Root is not a directory!");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("[DIR]  ");
      Serial.println(file.name());
    } else {
      Serial.print("[FILE] ");
      Serial.print(file.name());
      Serial.print("  (");
      Serial.print(file.size());
      Serial.println(" bytes)");
    }
    file = root.openNextFile();
  }
}


void setup() {
  Serial.begin(115200);
  // -----------------------------
  // I2C + TCA9554 + TOUCH BUS
  // -----------------------------
  // Must match LGFX touch pins: SDA=8, SCL=7
  Wire.begin(8, 7);
  tca.begin();
  reset_lcd_via_tca();

  // -----------------------------
  // LCD INIT
  // -----------------------------
  lcd.init();
  lcd.setRotation(1);          // 320x480, landscape
  lcd.fillScreen(TFT_BLACK);
  Serial.println("LCD initialized.");
  // -----------------------------
  // SD CARD INIT (SDIO 1-bit)
  // -----------------------------
  Serial.println("Initializing SD_MMC (SDIO 1-bit mode)...");

  // Factory demo pins:
  // clk = 11, cmd = 10, d0 = 9
  if (!SD_MMC.setPins(11, 10, 9)) {
    Serial.println("SD_MMC.setPins failed!");
  } else {
    if (!SD_MMC.begin("/sdcard", true)) {   // 1-bit mode
      Serial.println("SD_MMC.begin failed (Card Mount Failed)");
    } else {
      uint8_t cardType = SD_MMC.cardType();
      if (cardType == CARD_NONE) {
        Serial.println("No SD_MMC card attached");
      } else {
        Serial.println("SD_MMC initialized.");
      }
    }
  }

  // -----------------------------
  // UI STATE
  // -----------------------------
  cstate = SPLASH;

  // Draw splash
  API::DrawText("MTOS", 2, -68, TFT_DARKGRAY, TFT_BLACK, 7, true, true);
  API::DrawText("MTOS", 0, -70, TFT_WHITE,   TFT_BLACK, 7, true, true);
  API::DrawText("By: Manomay Tyagi 2026", 0, 0, TFT_WHITE, TFT_BLACK, 2, true, true);

  StartBtn.SetWidth();
  StartBtn.Draw();

  printHomeDir();

  wasm.Init();
}

void loop() {

  if (cstate == SPLASH)
  {
    if (StartBtn.CheckPress())
    {
      cstate = MENU;
      update = true;
    }
  }

  else if (cstate == MENU)
  {
    if (update)
    {
      Serial.println("Menu mode!");
      lcd.fillScreen(TFT_BLACK);

      // NOTE: SD_MMC is mounted at "/sdcard"
      // If your API::DrawRaw assumes root "/", make sure it uses SD_MMC and "/sdcard/..."
      API::DrawRaw("/os/img/install.raw", 17,  0, 64, 64);
      API::DrawRaw("/os/img/run.raw",     91,  0, 64, 64);
      API::DrawRaw("/os/img/sdv.raw",    165,  0, 64, 64);
      API::DrawRaw("/os/img/sett.raw",   239,  0, 64, 64);

      update = false;
    }

    if (install.CheckPress() || sdv.CheckPress() || sett.CheckPress())
    {
      cstate = RUN;
      update = true;
    }

    if (runBtn.CheckPress())
    {
      cstate = BOOT;
      update = true;
    }
  }

  else if (cstate == BOOT)
  {
    if (update)
    {
      lcd.fillScreen(TFT_BLACK);
      // TODO: draw boot animation / app loader
      update = false;
    }
  }

  else if (cstate == RUN)
  {
    if (update)
    {
      update = false;
      lcd.fillScreen(TFT_BLACK);
      // TODO: run selected app
      /*if(wasm.Run("/test.wasm"))
      {
        //go back to the Menu mode again (program quit)
        cstate = MENU;
        update = true;
      }
      else
      {
        Serial.println("MTOS: WASM execution failed, falling back to MENU mode...");
      }
    }*/
  }

  delay(50);
}
