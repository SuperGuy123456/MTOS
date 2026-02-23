#ifndef SDVIEWER_H
#define SDVIEWER_H

#include "API.h"
#include "FS.h"
#include "SD_MMC.h"

class SDViewerApp
{
public:
    void InitialDraw();
    void Update();
    void LoadDirectory(const String& path);

private:
    // --- PATH + DIRECTORY STATE ---
    String currentPath;
    File currentDir;

    // --- ENTRY BUFFER (48 entries) ---
    static const int MAX_ENTRIES = 48;

    struct Entry {
        String name;
        bool isDir;
        uint32_t size;
    };

    Entry entries[MAX_ENTRIES];
    int entryCount = 0;

    // --- SCROLLING + SELECTION ---
    int scrollIndex = 0;
    int selectedIndex = -1;

    // --- UI CONSTANTS ---
    const int rowHeight = 40;
    const int visibleRows = 5;

    // --- RIGHT-SIDE CONTROL COLUMN (420px display) ---
    const int parentX = 372, parentY = 0,   parentW = 80, parentH = 48;
    const int scrollUpX = 372, scrollUpY = 80, scrollUpW = 80, scrollUpH = 48;

    // DELETE BUTTON GOES HERE (between up and down)
    const int deleteX = 372, deleteY = 140, deleteW = 80, deleteH = 48;

    const int scrollDownX = 372, scrollDownY = 200, scrollDownW = 80, scrollDownH = 48;

    // File viewer back button
    const int backX = 340, backY = 0, backW = 80, backH = 48;

    // --- REDRAW FLAG ---
    bool needsRedraw = true;

    // --- INTERNAL HELPERS ---
    void DrawList();
    void DrawEntry(int index, int y);
    void HandleTouch();
    void EnterEntry(int index);
    String HumanSize(uint32_t bytes);

    void ShowFile(const String& fullpath);

    // NEW: deletion helpers
    void DeleteEntry(int index);
    bool ConfirmDelete(const String& name);
};

#endif
