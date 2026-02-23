#include <Arduino.h>
#include "LGFX_config.h"
#include "Button.h"
#include "Wasm.h"
#include <functional>
#include "API.h"
#include "wifi_secret.h"
#include "SDViewer.h"
#include "Installer.h"
#include "Uninstaller.h"
#include "Run.h"

#include <Wire.h>

#include "FS.h"
#include "SD_MMC.h"

// screen dimensions are 320x480 (panel), your UI uses 320x240 region
enum State
{
  SPLASH, //Start menu
  MENU, //Shows sys apps
  BOOT, //the run button triggers this
  SDVIEW, //The sd view button triggers this
  INSTALLR, //The install button triggers this
  UNINSTALLER,
  SETTING //The gear button triggers this (Does nothing right now...)
};

SimpleButton StartBtn("start", 190, 215, TFT_WHITE, TFT_RED, 3);

InvisButton install     (17,   0, 64, 64);
InvisButton uninstaller (91,   0, 64, 64);
InvisButton runBtn      (165,  0, 64, 64);
InvisButton sdv         (239,  0, 64, 64);
InvisButton sett        (313,  0, 64, 64);

InvisButton HomeBtn(0,0,48,48);


bool update = false;
State cstate;
String app_to_run;

TCA9554 tca(0x20);

Wasm wasm;

SDViewerApp sdvapp;
InstallerApp installerapp;
UninstallerApp uninstallerapp;
RunApp runapp;

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

bool BeginState()
{
    if (!update) return false;
    update = false;
    lcd.fillScreen(TFT_BLACK);
    API::DrawRaw("/os/img/Home.raw", 0, 0, 48, 48);
    return true;
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

  API::ConnectToWiFi(WIFI_SSID, WIFI_PASSWORD);
  if (!SD_MMC.exists("/manifest.txt"))
  {
    Serial.println("Manifest not found, downloading...");
    API::DownloadFile("https://raw.githubusercontent.com/SuperGuy123456/MTOSApps/main/manifest.txt", "/manifest.txt");
    delay(1000);
    lcd.fillScreen(TFT_BLACK);
  }
  else
  {
    if (!API::CheckManifestUpToDate())
    {
      Serial.println("Manifest is outdated, downloading...");
      API::DownloadFile("https://raw.githubusercontent.com/SuperGuy123456/MTOSApps/main/manifest.txt", "/manifest.txt");
      delay(1000);
      lcd.fillScreen(TFT_BLACK);
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

void loop() 
{

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
    if (BeginState())
    {
      Serial.println("Menu mode!");


      // NOTE: SD_MMC is mounted at "/sdcard"
      // If your API::DrawRaw assumes root "/", make sure it uses SD_MMC and "/sdcard/..."
      lcd.fillScreen(TFT_BLACK); //to hide the home button as it is not needed here...
      API::DrawRaw("/os/img/install.raw", 17,  0, 64, 64);
      API::DrawRaw("/os/img/uninstaller.raw", 91,  0, 64, 64);
      API::DrawRaw("/os/img/run.raw",     165,  0, 64, 64);
      API::DrawRaw("/os/img/sdviewer.raw",    239,  0, 64, 64);
      API::DrawRaw("/os/img/setting.raw",   313,  0, 64, 64);

    }

    if (install.CheckPress())
    {
      cstate = INSTALLR;
      update = true;
    }
    if (sdv.CheckPress())
    {
      cstate = SDVIEW;
      update = true;
    }
    if (sett.CheckPress())
    {
      cstate = SETTING;
      update = true;
    }
    if (runBtn.CheckPress())
    {
      cstate = BOOT;
      update = true;
    }
    if (uninstaller.CheckPress())
    {
      cstate = UNINSTALLER;
      update = true;
    }
  }

  else if (cstate == BOOT)
  {
    if (BeginState())
    {
      // TODO: show list
      runapp.InitialDraw();
    }
    if (HomeBtn.CheckPress())
    {
      cstate = MENU;
      update = true;
    }
    runapp.Update();
    API::DrawRaw("/os/img/Home.raw", 0, 0, 48, 48);

    //if button clicked, send to RUN
  }

  else if (cstate == INSTALLR)
  {
    if (BeginState())
    {
      // TODO: show list from manifest
      installerapp.InitialDraw();
    }
    if (HomeBtn.CheckPress())
    {
      cstate = MENU;
      update = true;
      return;
    }
    installerapp.Update();
    API::DrawRaw("/os/img/Home.raw", 0, 0, 48, 48);
  }

  else if (cstate == SETTING)
  {
    if (BeginState())
    {
      // TODO: show settings
    }
    if (HomeBtn.CheckPress())
    {
      cstate = MENU;
      update = true;
    }
  }

  else if (cstate == SDVIEW)
  {
      if (BeginState())
      {
          sdvapp.InitialDraw();
      }

      if (HomeBtn.CheckPress())
      {
          cstate = MENU;
          update = true;
          return;   // stop SDViewer from drawing again
      }

      sdvapp.Update();
      API::DrawRaw("/os/img/Home.raw", 0, 0, 48, 48);
  }

  else if (cstate == UNINSTALLER)
  {
    if (BeginState())
    {
      uninstallerapp.InitialDraw();
    }
    if (HomeBtn.CheckPress())
    {
      cstate = MENU;
      update = true;
    }
    uninstallerapp.Update();
    API::DrawRaw("/os/img/Home.raw", 0, 0, 48, 48);
  }

  delay(30);
}