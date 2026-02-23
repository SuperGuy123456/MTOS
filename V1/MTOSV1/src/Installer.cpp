#include "Installer.h"
#include "LGFX_config.h"

extern LGFX lcd;

void InstallerApp::InitialDraw()
{
    LoadAppList();
    lcd.fillScreen(TFT_BLACK);
    DrawScreen();
    needsRedraw = false;
}

void InstallerApp::Update()
{
    HandleTouch();

    if (needsRedraw)
    {
        lcd.fillScreen(TFT_BLACK);
        DrawScreen();
        needsRedraw = false;
    }
}

void InstallerApp::LoadAppList()
{
    appCount = 0;
    selectedApp = 0;

    File f = SD_MMC.open("/manifest.txt");
    if (!f)
    {
        // No manifest found
        return;
    }

    while (f.available() && appCount < MAX_APPS)
    {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) continue;

        // Expected format:
        // Name#Description#Version#WasmURL#Assets!
        int p1 = line.indexOf('#');
        int p2 = (p1 >= 0) ? line.indexOf('#', p1 + 1) : -1;
        int p3 = (p2 >= 0) ? line.indexOf('#', p2 + 1) : -1;

        if (p1 < 0 || p2 < 0) continue; // need at least name + description

        AppInfo info;
        info.name        = line.substring(0, p1);
        info.description = line.substring(p1 + 1, p2);

        if (p3 > p2)
            info.version = line.substring(p2 + 1, p3);
        else
            info.version = line.substring(p2 + 1);

        info.name.trim();
        info.description.trim();
        info.version.trim();

        if (info.name.length() == 0) continue;

        apps[appCount++] = info;
    }

    f.close();
}

void InstallerApp::DrawScreen()
{
    if (appCount == 0)
    {
        API::DrawText("No apps in manifest", 0, 80, TFT_RED, TFT_BLACK, 2, false, false);
        API::DrawText("/manifest.txt", 0, 110, TFT_LIGHTGREY, TFT_BLACK, 2, false, false);
        return;
    }

    const AppInfo& app = apps[selectedApp];

    // App name
    API::DrawText(app.name.c_str(), 0, 48, TFT_WHITE, TFT_BLACK, 3, false, false);

    // Install button (64x64 icon, right side)
    API::DrawRaw("/os/img/install.raw", installX, installY, 64, 64);

    // Version
    String ver = "Version: " + app.version;
    API::DrawText(ver.c_str(), 0, 100, TFT_LIGHTGREY, TFT_BLACK, 2, false, false);

    // Description (wrapped)
    DrawDescriptionWrapped(app.description, 0, 140, 350);

    // Scroll buttons
    API::DrawRaw("/os/img/left.raw",  scrollLeftX,  scrollLeftY,  48, 48);
    API::DrawRaw("/os/img/right.raw", scrollRightX, scrollRightY, 48, 48);
}

void InstallerApp::DrawDescriptionWrapped(const String& text, int x, int y, int maxWidth)
{
    String line;
    String word;

    for (int i = 0; i < text.length(); i++)
    {
        char c = text[i];

        if (c == ' ')
        {
            int w = lcd.textWidth((line + word).c_str());
            if (w > maxWidth)
            {
                API::DrawText(line.c_str(), x, y, TFT_WHITE, TFT_BLACK, 2, false, false);
                y += 20;
                line = word + " ";
            }
            else
            {
                line += word + " ";
            }
            word = "";
        }
        else
        {
            word += c;
        }
    }

    if (line.length() || word.length())
    {
        API::DrawText((line + word).c_str(), x, y, TFT_WHITE, TFT_BLACK, 2, false, false);
    }
}

void InstallerApp::HandleTouch()
{
    Vector2 t;
    if (!API::PollTouch(t)) return;

    int tx = t.x;
    int ty = t.y;

    if (appCount == 0) return;

    // Install button
    if (tx > installX && tx < installX + installW &&
        ty > installY && ty < installY + installH)
    {
        API::DownloadApp(apps[selectedApp].name.c_str());
        needsRedraw = true;
        return;
    }

    // Scroll left
    if (tx > scrollLeftX && tx < scrollLeftX + scrollLeftW &&
        ty > scrollLeftY && ty < scrollLeftY + scrollLeftH)
    {
        if (selectedApp > 0)
        {
            selectedApp--;
            needsRedraw = true;
        }
        return;
    }

    // Scroll right
    if (tx > scrollRightX && tx < scrollRightX + scrollRightW &&
        ty > scrollRightY && ty < scrollRightY + scrollRightH)
    {
        if (selectedApp < appCount - 1)
        {
            selectedApp++;
            needsRedraw = true;
        }
        return;
    }
}
