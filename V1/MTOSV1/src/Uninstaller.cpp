#include "Uninstaller.h"
#include "LGFX_config.h"

extern LGFX lcd;

void UninstallerApp::InitialDraw()
{
    LoadAppList();
    lcd.fillScreen(TFT_BLACK);
    DrawScreen();
    needsRedraw = false;
}

void UninstallerApp::Update()
{
    HandleTouch();

    if (needsRedraw)
    {
        lcd.fillScreen(TFT_BLACK);
        DrawScreen();
        needsRedraw = false;
    }
}

void UninstallerApp::LoadAppList()
{
    appCount = 0;
    selectedApp = 0;

    File f = SD_MMC.open("/apppaths.txt");
    if (!f)
    {
        return;
    }

    while (f.available() && appCount < MAX_APPS)
    {
        String folder = f.readStringUntil('\n');
        folder.trim();
        if (folder.length() == 0) continue;

        AppInfo info;
        info.folder = folder;

        // Load appinfo.txt
        String infoPath = folder + "/appinfo.txt";
        if (SD_MMC.exists(infoPath))
        {
            File af = SD_MMC.open(infoPath);
            if (af)
            {
                info.infoText = af.readString();
                af.close();
            }
            else
            {
                info.infoText = "Failed to read appinfo.txt";
            }
        }
        else
        {
            info.infoText = "No appinfo.txt found";
        }

        apps[appCount++] = info;
    }

    f.close();
}

void UninstallerApp::DrawScreen()
{
    if (appCount == 0)
    {
        API::DrawText("No installed apps", 0, 80, TFT_RED, TFT_BLACK, 2, false, false);
        API::DrawText("/apppaths.txt", 0, 110, TFT_LIGHTGREY, TFT_BLACK, 2, false, false);
        return;
    }

    const AppInfo& app = apps[selectedApp];

    // Folder name as title
    API::DrawText(app.folder.c_str(), 0, 48, TFT_WHITE, TFT_BLACK, 3, false, false);

    // Delete button
    API::DrawRaw("/os/img/uninstaller.raw", deleteX, deleteY, 64, 64);

    // Description (raw appinfo.txt)
    DrawDescriptionWrapped(app.infoText, 0, 140, 350);

    // Scroll buttons
    API::DrawRaw("/os/img/left.raw",  scrollLeftX,  scrollLeftY,  48, 48);
    API::DrawRaw("/os/img/right.raw", scrollRightX, scrollRightY, 48, 48);
}

void UninstallerApp::DrawDescriptionWrapped(const String& text, int x, int y, int maxWidth)
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

void UninstallerApp::HandleTouch()
{
    Vector2 t;
    if (!API::PollTouch(t)) return;

    int tx = t.x;
    int ty = t.y;

    if (appCount == 0) return;

    // Delete button
    if (tx > deleteX && tx < deleteX + deleteW &&
        ty > deleteY && ty < deleteY + deleteH)
    {
        String folder = apps[selectedApp].folder;

        // 1. Delete folder
        API::RecursiveDelete(folder.c_str());

        // 2. Remove from apppaths.txt
        std::vector<String> remaining;

        File f = SD_MMC.open("/apppaths.txt", FILE_READ);
        if (f) {
            while (f.available()) {
                String line = f.readStringUntil('\n');
                line.trim();
                if (line.length() == 0) continue;

                if (line != folder) {
                    remaining.push_back(line);
                }
            }
            f.close();
        }

        File out = SD_MMC.open("/apppaths.txt", FILE_WRITE);
        if (out) {
            for (auto& p : remaining) {
                out.println(p);
            }
            out.close();
        }

        // 3. Reload UI
        LoadAppList();
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
