#  MTOS
*© 2025 Manomay Tyagi*

## 📖 About the Project

**MTOS** is essentially a TINY os that runs off of the esp32 flash and it has the capability to run wasm files from the sd card by. passing the flash size limits of the board and also, eliminating the need to reflash to run different programs. It allows the user to download multiple projects onto the sd card and run them at runtime without the need of plugging into a computer.

## 🚀 Trying It Out

Unless you own an **WaveShare ESP32-S3-Touch-LCD-3.5-C** with the exact same pin configuration required by this project, you'll need to download the project files and modify the pin assignments and possibly the libraries. (Please look at the Porting.md file)

For those using the **WaveShare ESP32-S3-Touch-LCD-3.5-C**, you can simply download the project files, flash them to your ESP32, and download the micro sd card file tree for all of the assets.

## 📌 Features

- Downloads `.wasm` files (And additional asset files) from a remote HTTP/HTTPS server  
- Saves the downloaded file to an SD card  
- Displays available files via touchscreen interface  
- Reads and Runs the selected wasm from the SD card  
- Automatically goes back into OS mode after running

## 🧰 Requirements

- **WaveShare ESP32-S3-Touch-LCD-3.5-C** ("Cheap Yellow Display")  
- MicroSD card formatted as `FAT32` (recommended size < 16GB) and MUST contain the required files
- Wi-Fi access (you must create a wifi_secret.h) file (please refer to Porting.md)
- `.wasm` files hosted online with similar manifest

## 🔄 Update Flow

1. ESP32 connects to Wi-Fi  
2. It downloads the `.wasm` file from a specified URL  
3. The file is saved to the SD card with appropriate folder structure
4. Updates internal App registery
5. The ESP32 reads the file from SD and runs it using `Wasm` class 
6. After running fails or succeeds, exit back into os. 

## Porting

Please refer to the porting.md file for a slightly incomplete checklist on how to port MTOS to different platforms

## 🧑‍⚖️ License

**This project is protected by the Creative Commons Attribution-NonCommercial 4.0 International License**

*© 2025 Manomay Tyagi*
____
