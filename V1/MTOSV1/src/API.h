#pragma once
#include "LGFX_config.h"
#include "SD_MMC.h"
#include "WiFi.h"
#include "HTTPClient.h"

extern LGFX lcd;

struct Vector2
{
    int32_t x;
    int32_t y;
};

class API {
public:
    //Functions to do with screen
    static void DrawText(const char* text, int x, int y,
                       int fgcolor, int bgcolor,
                       uint8_t size,
                       bool nobg=false, bool centered=false);//draws text
    static bool DrawRaw(const char* path, int x, int y, int w, int h);//Draws a .raw file from sdcard
    static bool DrawPixel(int x, int y, int color); //Exposes the barebones pixel for apps that want to do their own drawing
    static bool ClearScreen(int color); //For screen refreshes
    static bool DrawRect(int x, int y, int w, int h, int color); //Draws a rectangle, for buttons and stuff
    static bool FillRect(int x, int y, int w, int h, int color); //Draws a filled rectangle, for buttons and stuff
    static bool DrawCircle(int x, int y, int r, int color); //Draws a circle, for buttons and stuff
    static bool FillCircle(int x, int y, int r, int color); //Draws a filled circle, for buttons and stuff
    
    //Functions to do with peripherals
    static bool PollTouch(Vector2& coord);//spits out coords and true for touch

    //Functions to do with WiFi and downloading
    static bool ConnectToWiFi(const char* ssid, const char* password); //connects to wifi, returns true if successful
    static bool DownloadFile(const char* url,  const char* localPath); //downloads any file from internet
    static bool DownloadApp(const char* name); //Downloads from a file called manifest.txt
    static bool CheckManifestUpToDate(); //Checks if manifest.txt is up to date by comparing it to the one on github, returns true if up to date

    //Functions to do with file system
    static bool RecursiveDelete(const char* path);
    static bool FileExists(const char* path);
    static bool ReadFile(const char* path, char* buffer, size_t bufferSize);
    static bool WriteFile(const char* path, const char* data);
    static bool AppendToFile(const char* path, const char* data);
    static bool ListDirectory(const char* path); //For debugging, lists files in a directory to serial
    static bool CreateDirectory(const char* path);

    //Time and delays
    static uint32_t GetTime(); //Returns time in seconds since epoch
    static void Delay(uint32_t ms); //Delays for a specified number of milliseconds 

    //Randomness
    static int Random(int min, int max); //Returns a random integer between min and max (inclusive)
};