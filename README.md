# MTOS
*© 2025 Manomay Tyagi*

## 📖 About the Project

**MTOS** is a tiny operating system that runs entirely from the ESP32’s internal flash and executes WebAssembly (`.wasm`) apps stored on a microSD card.  
This bypasses the ESP32’s limited flash size and eliminates the need to reflash the board every time you want to run a different program.

Users can download multiple apps onto the SD card and run them at runtime — no USB cable required.

---

## 🚀 Trying It Out

If you are **not** using the **WaveShare ESP32-S3-Touch-LCD-3.5-C**, you will need to modify pin assignments and possibly some library configurations.  
See **Porting.md** for details.

If you *are* using the WaveShare board, simply:

1. Download the project files  
2. Flash the firmware to your ESP32-S3  
3. Prepare the microSD card with the required folder structure  
4. Boot the device  

---

## 📌 Features

- Downloads `.wasm` apps (and optional asset files) from remote HTTP/HTTPS servers  
- Saves downloaded files to the SD card  
- Displays available apps via touchscreen interface  
- Loads and runs `.wasm` apps directly from SD  
- Automatically returns to OS mode after the app exits or fails  

---

## 🧰 Requirements

- **WaveShare ESP32-S3-Touch-LCD-3.5-C**  
- MicroSD card formatted as **FAT32** (recommended < 16GB)  
- Wi-Fi access (requires a `wifi_secret.h` — see Porting.md)  
- `.wasm` files hosted online with a compatible manifest format  

---

## 🔄 Update Flow

1. ESP32 connects to Wi-Fi  
2. Downloads the `.wasm` file from a specified URL  
3. Saves the file to the SD card with the correct folder structure  
4. Updates the internal app registry  
5. Loads and runs the app using the MTOS WASM runtime  
6. Returns to OS mode after the app exits  

---

## 🛠️ Porting

See **Porting.md** for a (currently incomplete) checklist on how to port MTOS to other ESP32-S3 boards or displays.

---

## ⚙️ Compiling

**This is what worked for me, it might not work for you though...**
```
emcc -O2 test.cpp -o test.wasm -s STANDALONE_WASM=1 -s INITIAL_MEMORY=6291456 -s ERROR_ON_UNDEFINED_SYMBOLS=0
```

## 🧑‍⚖️ License

**Creative Commons Attribution-NonCommercial 4.0 International License**

*© 2025 Manomay Tyagi*
