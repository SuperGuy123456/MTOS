#ifndef RUNAPP_H
#define RUNAPP_H

#include "API.h"
#include "FS.h"
#include "SD_MMC.h"
#include "Wasm.h"

class RunApp
{
public:
    void InitialDraw();
    void Update();

private:
    struct AppInfo {
        String folder;      // "/Clock"
        String infoText;    // raw contents of appinfo.txt
    };

    static const int MAX_APPS = 16;
    AppInfo apps[MAX_APPS];
    int appCount = 0;
    int selectedApp = 0;

    // UI constants (same layout)
    const int runX = 372, runY = 48, runW = 80, runH = 48;
    const int scrollLeftX = 0, scrollLeftY = 260, scrollLeftW = 80, scrollLeftH = 48;
    const int scrollRightX = 372, scrollRightY = 260, scrollRightW = 80, scrollRightH = 48;

    bool needsRedraw = true;

    void DrawScreen();
    void DrawDescriptionWrapped(const String& text, int x, int y, int maxWidth);
    void HandleTouch();
    void LoadAppList();
};

#endif
