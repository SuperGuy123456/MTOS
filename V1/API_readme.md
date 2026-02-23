# MTOS API v1
*Stable WebAssembly → Native Interface*  
*© 2025 Manomay Tyagi*

MTOS exposes a minimal, stable, and game‑friendly API to WebAssembly apps.  
All functions are imported under the **`env`** module and are guaranteed to remain compatible across future OS versions.

This document describes every available function, its purpose, and its WASM signature.

---

## 📺 Screen & Drawing

### **`void DrawText(char* text, int x, int y, int fg, int bg, int size, int nobg, int centered)`**
Draws text on the screen.

- `text` — null‑terminated UTF‑8 string  
- `x, y` — position  
- `fg, bg` — RGB565 colors  
- `size` — text scale  
- `nobg` — 1 = transparent background  
- `centered` — 1 = center text relative to screen  

**WASM signature:** `v(*iiiiiii)`

---

### **`int DrawPixel(int x, int y, int color)`**
Draws a single pixel.

**WASM signature:** `i(iii)`

---

### **`int ClearScreen(int color)`**
Fills the entire screen with a color.

**WASM signature:** `i(i)`

---

### **`int DrawRect(int x, int y, int w, int h, int color)`**
Draws an outline rectangle.

**WASM signature:** `i(iiiii)`

---

### **`int FillRect(int x, int y, int w, int h, int color)`**
Draws a filled rectangle.

**WASM signature:** `i(iiiii)`

---

### **`int DrawCircle(int x, int y, int r, int color)`**
Draws a circle outline.

**WASM signature:** `i(iiii)`

---

### **`int FillCircle(int x, int y, int r, int color)`**
Draws a filled circle.

**WASM signature:** `i(iiii)`

---

### **`int DrawRaw(char* path, int x, int y, int w, int h)`**
Draws a `.raw` RGB565 image from the SD card.

**WASM signature:** `i(*iiiii)`

---

## 👆 Touch Input

### **`int PollTouch(int* outPtr)`**
Reads touchscreen input.

- Writes `x` to `outPtr[0]`
- Writes `y` to `outPtr[1]`
- Returns `1` if touched, `0` otherwise

**WASM signature:** `i(*)`

---

## 🌐 WiFi & Downloading

### **`int ConnectToWiFi(char* ssid, char* password)`**
Connects to a WiFi network.

**WASM signature:** `i(**)`

---

### **`int DownloadFile(char* url, char* localPath)`**
Downloads a file from HTTP/HTTPS and saves it to the SD card.

**WASM signature:** `i(**)`

---

### **`int DownloadApp(char* name)`**
Downloads an app defined in `manifest.txt`.

**WASM signature:** `i(*)`

---

### **`int CheckManifestUpToDate()`**
Checks whether the local manifest matches the remote one.

**WASM signature:** `i()`

---

## 📁 Filesystem

### **`int RecursiveDelete(char* path)`**
Deletes a directory and all its contents.

**WASM signature:** `i(*)`

---

### **`int FileExists(char* path)`**
Returns `1` if the file exists.

**WASM signature:** `i(*)`

---

### **`int ReadFile(char* path, char* buffer, int bufferSize)`**
Reads a file into a buffer and null‑terminates it.

**WASM signature:** `i(* * i)`

---

### **`int WriteFile(char* path, char* data)`**
Writes text to a file.

**WASM signature:** `i(**)`

---

### **`int AppendToFile(char* path, char* data)`**
Appends text to a file.

**WASM signature:** `i(**)`

---

### **`int ListDirectory(char* path)`**
Prints directory contents to serial.

**WASM signature:** `i(*)`

---

### **`int CreateDirectory(char* path)`**
Creates a directory.

**WASM signature:** `i(*)`

---

## ⏱️ Time

### **`int GetTime()`**
Returns seconds since Unix epoch.

**WASM signature:** `i()`

---

### **`void Delay(int ms)`**
Pauses execution for a number of milliseconds.

**WASM signature:** `v(i)`

---

## 🎲 Random

### **`int Random(int min, int max)`**
Returns a random integer in the range `[min, max)`.

**WASM signature:** `i(ii)`

---

## Example Usage in wasm file:

```
extern "C" {

extern void DrawText(const char* text,
                     int x, int y,
                     int fgcolor, int bgcolor,
                     unsigned char size,
                     bool nobg, bool centered);
}

int main() {  // Changed from app_main
    DrawText("Hello World",
             0, 0,
             0xFFFFFF,
             0x000000,
             2,
             true,
             false);
    return 0;
}
```