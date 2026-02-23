#include "SDViewer.h"
#include "API.h"
#include "LGFX_config.h"
#include "FS.h"
#include "SD_MMC.h"

extern LGFX lcd;

// Convert bytes → human readable
String SDViewerApp::HumanSize(uint32_t bytes) {
    if (bytes < 1024) return String(bytes) + " B";
    if (bytes < 1024 * 1024) return String(bytes / 1024.0, 1) + " KB";
    return String(bytes / 1048576.0, 1) + " MB";
}

void SDViewerApp::InitialDraw()
{
    currentPath = "/";
    scrollIndex = 0;
    selectedIndex = -1;

    LoadDirectory(currentPath);

    lcd.fillScreen(TFT_BLACK);

    API::DrawText(currentPath.c_str(), 0, 48, TFT_WHITE, TFT_BLACK, 2, false, false);

    // Right-side buttons
    API::DrawRaw("/os/img/sdviewer/updir.raw", parentX, parentY, 48, 48);
    API::DrawRaw("/os/img/up.raw",   scrollUpX,  scrollUpY,  48, 48);
    API::DrawRaw("/os/img/trash.raw", deleteX, deleteY, 48, 48);
    API::DrawRaw("/os/img/down.raw", scrollDownX, scrollDownY, 48, 48);

    DrawList();
    needsRedraw = false;
}

void SDViewerApp::Update()
{
    HandleTouch();

    if (needsRedraw)
    {
        lcd.fillScreen(TFT_BLACK);

        API::DrawText(currentPath.c_str(), 0, 48, TFT_WHITE, TFT_BLACK, 2, false, false);

        API::DrawRaw("/os/img/sdviewer/updir.raw", parentX, parentY, 48, 48);
        API::DrawRaw("/os/img/up.raw",   scrollUpX,  scrollUpY,  48, 48);
        API::DrawRaw("/os/img/trash.raw", deleteX, deleteY, 48, 48);
        API::DrawRaw("/os/img/down.raw", scrollDownX, scrollDownY, 48, 48);

        DrawList();
        needsRedraw = false;
    }
}

void SDViewerApp::LoadDirectory(const String& path)
{
    if (currentDir) currentDir.close();

    currentDir = SD_MMC.open(path);
    if (!currentDir || !currentDir.isDirectory())
    {
        entryCount = 0;
        return;
    }

    currentPath = path;
    entryCount = 0;

    File f = currentDir.openNextFile();
    while (f && entryCount < MAX_ENTRIES)
    {
        entries[entryCount].name = f.name();
        entries[entryCount].isDir = f.isDirectory();
        entries[entryCount].size = f.size();
        entryCount++;

        f = currentDir.openNextFile();
    }

    scrollIndex = 0;
    selectedIndex = -1;
    needsRedraw = true;
}

void SDViewerApp::DrawList()
{
    int y = 80;

    for (int i = 0; i < visibleRows; i++)
    {
        int idx = scrollIndex + i;
        if (idx >= entryCount) break;

        DrawEntry(idx, y);
        y += rowHeight;
    }
}

void SDViewerApp::DrawEntry(int index, int y)
{
    const Entry& e = entries[index];

    if (index == selectedIndex)
        lcd.fillRect(0, y, 350, rowHeight, TFT_DARKGREY);

    // 16x16 icons
    if (e.isDir)
        API::DrawRaw("/os/img/sdviewer/folder.raw", 0, y + 12, 16, 16);
    else
        API::DrawRaw("/os/img/sdviewer/file.raw", 0, y + 12, 16, 16);

    // Filename
    API::DrawText(e.name.c_str(), 24, y + 8, TFT_WHITE, TFT_BLACK, 2, false, false);

    // File size (only for files)
    if (!e.isDir)
    {
        String sizeStr = HumanSize(e.size);
        int textWidth = lcd.textWidth(sizeStr);
        int sx = 300 - textWidth;
        API::DrawText(sizeStr.c_str(), sx, y + 8, TFT_LIGHTGREY, TFT_BLACK, 2, false, false);
    }
}

void SDViewerApp::HandleTouch()
{
    Vector2 t;
    bool pressed = API::PollTouch(t);
    if (!pressed) return;

    int tx = t.x;
    int ty = t.y;

    // Parent directory button
    if (tx > parentX && tx < parentX + parentW &&
        ty > parentY && ty < parentY + parentH)
    {
        if (currentPath != "/")
        {
            int slash = currentPath.lastIndexOf('/');
            if (slash > 0)
                LoadDirectory(currentPath.substring(0, slash));
            else
                LoadDirectory("/");
        }
        return;
    }

    // Scroll up
    if (tx > scrollUpX && tx < scrollUpX + scrollUpW &&
        ty > scrollUpY && ty < scrollUpY + scrollUpH)
    {
        if (scrollIndex > 0)
        {
            scrollIndex--;
            needsRedraw = true;
        }
        return;
    }

    // DELETE button
    if (tx > deleteX && tx < deleteX + deleteW &&
        ty > deleteY && ty < deleteY + deleteH)
    {
        if (selectedIndex >= 0)
            DeleteEntry(selectedIndex);
        return;
    }

    // Scroll down
    if (tx > scrollDownX && tx < scrollDownX + scrollDownW &&
        ty > scrollDownY && ty < scrollDownY + scrollDownH)
    {
        if (scrollIndex < entryCount - visibleRows)
        {
            scrollIndex++;
            needsRedraw = true;
        }
        return;
    }

    // List touch (must be inside list X area)
    if (ty >= 80 && tx < 340)
    {
        int row = (ty - 80) / rowHeight;
        if (row >= 0 && row < visibleRows)
        {
            int idx = scrollIndex + row;
            if (idx >= 0 && idx < entryCount)
            {
                if (selectedIndex == idx)
                    EnterEntry(idx);
                else
                {
                    selectedIndex = idx;
                    needsRedraw = true;
                }
            }
        }
    }
}

void SDViewerApp::ShowFile(const String& fullpath)
{
    lcd.fillScreen(TFT_BLACK);

    // Back button
    API::DrawRaw("/os/img/sdviewer/updir.raw", backX, backY, 48, 48);

    API::DrawText(fullpath.c_str(), 0, 48, TFT_WHITE, TFT_BLACK, 2, false, false);

    File f = SD_MMC.open(fullpath);
    if (!f)
    {
        API::DrawText("Failed to open file", 0, 80, TFT_RED, TFT_BLACK, 2, false, false);
        return;
    }

    int y = 80;
    while (f.available())
    {
        String line = f.readStringUntil('\n');
        API::DrawText(line.c_str(), 0, y, TFT_WHITE, TFT_BLACK, 2, false, false);
        y += 20;
        if (y > 460) break;
    }

    f.close();

    // Wait for BACK button press
    while (true)
    {
        Vector2 t;
        if (API::PollTouch(t))
        {
            if (t.x > backX && t.x < backX + backW &&
                t.y > backY && t.y < backY + backH)
            {
                needsRedraw = true;
                return;
            }
        }
        delay(20);
    }
}

bool SDViewerApp::ConfirmDelete(const String& name)
{
    lcd.fillScreen(TFT_BLACK);

    API::DrawText("Delete:", 0, 40, TFT_RED, TFT_BLACK, 3, false, false);
    API::DrawText(name.c_str(), 0, 80, TFT_WHITE, TFT_BLACK, 2, false, false);

    API::DrawText("Tap LEFT = Delete", 0, 160, TFT_LIGHTGREY, TFT_BLACK, 2, false, false);
    API::DrawText("Tap RIGHT = Cancel", 0, 200, TFT_LIGHTGREY, TFT_BLACK, 2, false, false);

    Vector2 t;

    // ⭐ IMPORTANT: Wait for user to RELEASE the touch
    while (API::PollTouch(t))
        delay(10);

    // ⭐ Now wait for a NEW touch
    while (true)
    {
        if (API::PollTouch(t))
        {
            if (t.x < 210) return true;   // LEFT = DELETE
            if (t.x >= 210) return false; // RIGHT = CANCEL
        }
        delay(20);
    }
}

void SDViewerApp::DeleteEntry(int index)
{
    if (index < 0 || index >= entryCount) return;

    Entry& e = entries[index];

    String fullpath = currentPath;
    if (!fullpath.endsWith("/")) fullpath += "/";
    fullpath += e.name;

    if (!ConfirmDelete(e.name))
    {
        needsRedraw = true;
        return;
    }

    if (e.isDir)
        API::RecursiveDelete(fullpath.c_str());
    else
        SD_MMC.remove(fullpath);

    LoadDirectory(currentPath);
}

void SDViewerApp::EnterEntry(int index)
{
    if (index < 0 || index >= entryCount) return;

    const Entry& e = entries[index];

    if (e.isDir)
    {
        String newPath = currentPath;
        if (!newPath.endsWith("/")) newPath += "/";
        newPath += e.name;

        LoadDirectory(newPath);
    }
    else
    {
        String fullpath = currentPath;
        if (!fullpath.endsWith("/")) fullpath += "/";
        fullpath += e.name;

        ShowFile(fullpath);
    }
}
