#ifndef UNINSTALLER_H
#define UNINSTALLER_H

#include "API.h"
#include "FS.h"
#include "SD_MMC.h"

class UninstallerApp
{
public:
    void InitialDraw();
    void Update();

private:
    // --- APP LIST ---
    struct AppInfo {
        String folder;      // "/Clock"
        String infoText;    // raw contents of appinfo.txt
    };

    static const int MAX_APPS = 16;
    AppInfo apps[MAX_APPS];
    int appCount = 0;
    int selectedApp = 0;

    // --- UI CONSTANTS ---
    const int deleteX = 372, deleteY = 48, deleteW = 80, deleteH = 48;
    const int scrollLeftX = 0, scrollLeftY = 260, scrollLeftW = 80, scrollLeftH = 48;
    const int scrollRightX = 372, scrollRightY = 260, scrollRightW = 80, scrollRightH = 48;

    bool needsRedraw = true;

    // --- INTERNAL HELPERS ---
    void DrawScreen();
    void DrawDescriptionWrapped(const String& text, int x, int y, int maxWidth);
    void HandleTouch();
    void LoadAppList(); // loads from apppaths.txt
};

#endif
