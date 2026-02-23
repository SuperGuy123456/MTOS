#ifndef INSTALLER_H
#define INSTALLER_H

#include "API.h"
#include "FS.h"
#include "SD_MMC.h"

class InstallerApp
{
public:
    void InitialDraw();
    void Update();

private:
    // --- APP LIST ---
    struct AppInfo {
        String name;
        String version;
        String description;
    };

    static const int MAX_APPS = 16;
    AppInfo apps[MAX_APPS];
    int appCount = 0;
    int selectedApp = 0;

    // --- UI CONSTANTS ---
    const int installX = 372, installY = 48, installW = 80, installH = 48;
    const int scrollLeftX = 0, scrollLeftY = 260, scrollLeftW = 80, scrollLeftH = 48;
    const int scrollRightX = 372, scrollRightY = 260, scrollRightW = 80, scrollRightH = 48;

    bool needsRedraw = true;

    // --- INTERNAL HELPERS ---
    void DrawScreen();
    void DrawDescriptionWrapped(const String& text, int x, int y, int maxWidth);
    void HandleTouch();
    void LoadAppList(); // loads from manifest list or static list
};

#endif
