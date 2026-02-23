#include "API.h"

// SCREEN and drawing
void API::DrawText(const char* text, int x, int y,
                   int fgcolor, int bgcolor,
                   uint8_t size,
                   bool nobg, bool centered)
{
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

bool API::DrawRaw(const char* path, int x, int y, int w, int h)
{
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
        if (bytesRead != (size_t)(w * 2)) {
            Serial.printf("Unexpected EOF in %s at row %d\n", path, row);
            free(lineBuf);
            f.close();
            return false;
        }

        lcd.pushImage(x, y + row, w, 1, lineBuf);
    }

    free(lineBuf);
    f.close();
    return true;
}

bool API::DrawPixel(int x, int y, int color)
{
    lcd.drawPixel(x, y, color);
    return true;
}

bool API::ClearScreen(int color)
{
    lcd.fillScreen(color);
    return true;
}

bool API::DrawRect(int x, int y, int w, int h, int color)
{
    lcd.drawRect(x, y, w, h, color);
    return true;
}

bool API::FillRect(int x, int y, int w, int h, int color)
{
    lcd.fillRect(x, y, w, h, color);
    return true;
}

bool API::DrawCircle(int x, int y, int r, int color)
{
    lcd.drawCircle(x, y, r, color);
    return true;
}

bool API::FillCircle(int x, int y, int r, int color)
{
    lcd.fillCircle(x, y, r, color);
    return true;
}

// PERIPHERALS
bool API::PollTouch(Vector2& coord)
{
    // Adjust this to your actual touch driver
    uint16_t tx, ty;
    if (lcd.getTouch(&tx, &ty)) {
        coord.x = (int32_t)tx;
        coord.y = (int32_t)ty;
        return true;
    }
    return false;
}

// WIFI and downloading
bool API::ConnectToWiFi(const char* ssid, const char* password)
{
    WiFi.begin(ssid, password);
    Serial.printf("Connecting to Wi-Fi: %s\n", ssid);

    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 20) {
        delay(500);
        Serial.print(".");
        retries++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("\nConnected! IP: %s\n", WiFi.localIP().toString().c_str());
        return true;
    } else {
        Serial.println("\nFailed to connect to Wi-Fi.");
        return false;
    }
}

bool API::DownloadFile(const char* url, const char* localPath)
{
    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("HTTP GET failed: %d\n", httpCode);
        http.end();
        API::DrawText("Download failed!", 0, 0, TFT_RED, TFT_BLACK, 3, true, true);
        return false;
    }

    int contentLength = http.getSize();
    WiFiClient* stream = http.getStreamPtr();

    File file = SD_MMC.open(localPath, FILE_WRITE);
    if (!file) {
        Serial.printf("Failed to open file: %s\n", localPath);
        http.end();
        API::DrawText("SD open failed!", 0, 0, TFT_RED, TFT_BLACK, 3, true, true);
        return false;
    }

    API::DrawText("Downloading...", 0, 0, TFT_WHITE, TFT_BLACK, 3, true, true);
    lcd.drawRect(50, 100, 220, 20, TFT_WHITE);

    uint8_t buf[2048];
    int totalRead = 0;
    unsigned long lastReadTime = millis();
    const unsigned long timeout = 10000;

    while (totalRead < contentLength) {
        int len = stream->readBytes(buf, sizeof(buf));
        if (len <= 0) {
            if (millis() - lastReadTime > timeout) {
                Serial.println("Timeout: No data received for 10 seconds.");
                break;
            }
            delay(100);
            continue;
        }

        lastReadTime = millis();
        size_t writtenNow = file.write(buf, len);
        if (writtenNow != (size_t)len) {
            Serial.println("SD write error.");
            break;
        }

        totalRead += len;
        Serial.printf("%d/%d bytes written\n", totalRead, contentLength);

        int progress = map(totalRead, 0, contentLength, 0, 218);
        lcd.fillRect(51, 101, progress, 18, TFT_GREEN);
    }

    file.close();
    http.end();

    if (totalRead == contentLength) {
        Serial.printf("Download complete: %d bytes written\n", totalRead);
        API::DrawText("Download complete!", 0, 100, TFT_GREEN, TFT_BLACK, 3, true, true);
        lcd.fillScreen(TFT_BLACK);
        return true;
    } else {
        Serial.printf("Incomplete download: %d/%d bytes written\n", totalRead, contentLength);
        API::DrawText("Download incomplete!", 0, 0, TFT_RED, TFT_BLACK, 3, true, true);
        return false;
    }
}

bool API::DownloadApp(const char* name)
{
    File f = SD_MMC.open("/manifest.txt", FILE_READ);
    if (!f) {
        Serial.println("Could not open manifest.txt");
        return false;
    }

    Serial.println("Reading manifest.txt...");

    while (f.available()) {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) continue;

        Serial.println("Checking line:");
        Serial.println(line);

        int sep1 = line.indexOf('#');
        int sep2 = line.indexOf('#', sep1 + 1);
        int sep3 = line.indexOf('#', sep2 + 1);
        int sep4 = line.indexOf('#', sep3 + 1);
        int end  = line.indexOf('!', sep4 + 1);

        if (sep1 == -1 || sep2 == -1 || sep3 == -1 || sep4 == -1 || end == -1) {
            Serial.println("Skipping malformed manifest line");
            continue;
        }

        String appName     = line.substring(0, sep1);
        String description = line.substring(sep1 + 1, sep2);
        String version     = line.substring(sep2 + 1, sep3);
        String wasmLink    = line.substring(sep3 + 1, sep4);
        String assetStr    = line.substring(sep4 + 1, end);

        appName.trim();
        description.trim();
        version.trim();
        wasmLink.trim();
        assetStr.trim();

        if (appName != name) {
            continue;
        }

        Serial.println("Found matching app: " + appName);

        std::vector<String> assets;
        if (assetStr != "N/A" && assetStr.length() > 0) {
            int start = 0;
            while (start < assetStr.length()) {
                int comma = assetStr.indexOf(',', start);
                if (comma == -1) {
                    assets.push_back(assetStr.substring(start));
                    break;
                }
                assets.push_back(assetStr.substring(start, comma));
                start = comma + 1;
            }
        }

        String basePath = "/" + appName;
        SD_MMC.mkdir(basePath);

        // appinfo.txt
        {
            File rf = SD_MMC.open(basePath + "/appinfo.txt", FILE_WRITE);
            if (!rf) {
                Serial.println("Failed to write appinfo.txt");
                f.close();
                return false;
            }
            rf.print(appName + "\n" + version + "\n" + description);
            rf.close();
        }

        if (!assets.empty()) {
            SD_MMC.mkdir(basePath + "/assets");
        }

        String wasmPath = basePath + "/app.wasm";
        Serial.println("Downloading WASM to: " + wasmPath);
        if (!DownloadFile(wasmLink.c_str(), wasmPath.c_str())) {
            Serial.println("WASM download failed");
            f.close();
            return false;
        }

        for (auto& a : assets) {
            int lastSlash = a.lastIndexOf('/');
            String filename = a.substring(lastSlash + 1);
            String local = basePath + "/assets/" + filename;

            Serial.println("Downloading asset: " + filename);
            if (!DownloadFile(a.c_str(), local.c_str())) {
                Serial.println("Asset download failed");
                f.close();
                return false;
            }
        }

        // Register app path in /apppaths.txt
        {
            String folderPath = basePath;

            bool alreadyExists = false;
            if (SD_MMC.exists("/apppaths.txt")) {
                File check = SD_MMC.open("/apppaths.txt", FILE_READ);
                if (check) {
                    while (check.available()) {
                        String line = check.readStringUntil('\n');
                        line.trim();
                        if (line == folderPath) {
                            alreadyExists = true;
                            break;
                        }
                    }
                    check.close();
                }
            }

            if (alreadyExists) {
                Serial.println("App folder already registered, skipping append.");
            } else {
                File pf = SD_MMC.open("/apppaths.txt", FILE_APPEND);
                if (!pf) {
                    Serial.println("Failed to open apppaths.txt for append");
                    f.close();
                    return false;
                }
                pf.println(folderPath);
                pf.close();
                Serial.println("Registered new app folder: " + folderPath);
            }
        }

        Serial.println("App installed successfully!");
        f.close();
        return true;
    }

    f.close();
    Serial.println("App not found in manifest");
    return false;
}

bool API::CheckManifestUpToDate()
{
    // Placeholder: implement your actual check logic here
    // e.g., compare remote version vs local version file
    return true;
}

// FILE SYSTEM
bool API::RecursiveDelete(const char* path)
{
    File dir = SD_MMC.open(path);
    if (!dir || !dir.isDirectory()) return false;

    File f = dir.openNextFile();
    while (f) {
        String child = String(path) + "/" + f.name();
        bool isDir = f.isDirectory();
        f.close();

        if (isDir)
            API::RecursiveDelete(child.c_str());
        else
            SD_MMC.remove(child);

        f = dir.openNextFile();
    }

    dir.close();
    return SD_MMC.rmdir(path);
}

bool API::FileExists(const char* path)
{
    return SD_MMC.exists(path);
}

bool API::ReadFile(const char* path, char* buffer, size_t bufferSize)
{
    if (bufferSize == 0) return false;

    File f = SD_MMC.open(path, FILE_READ);
    if (!f) return false;

    size_t bytesRead = f.readBytes(buffer, bufferSize - 1);
    buffer[bytesRead] = '\0';

    f.close();
    return true;
}

bool API::WriteFile(const char* path, const char* data)
{
    File f = SD_MMC.open(path, FILE_WRITE);
    if (!f) return false;

    f.print(data);
    f.close();
    return true;
}

bool API::AppendToFile(const char* path, const char* data)
{
    File f = SD_MMC.open(path, FILE_APPEND);
    if (!f) return false;

    f.print(data);
    f.close();
    return true;
}

bool API::ListDirectory(const char* path)
{
    File dir = SD_MMC.open(path);
    if (!dir || !dir.isDirectory()) return false;

    Serial.printf("Listing directory: %s\n", path);

    File f = dir.openNextFile();
    while (f) {
        if (f.isDirectory())
            Serial.printf("  [DIR]  %s\n", f.name());
        else
            Serial.printf("  [FILE] %s  (%d bytes)\n", f.name(), f.size());

        f.close();
        f = dir.openNextFile();
    }

    dir.close();
    return true;
}

bool API::CreateDirectory(const char* path)
{
    return SD_MMC.mkdir(path);
}

// TIME and delays
uint32_t API::GetTime()
{
    return (uint32_t)time(nullptr);
}

void API::Delay(uint32_t ms)
{
    delay(ms);
}

// RANDOMNESS
int API::Random(int min, int max)
{
    return random(min, max);
}
