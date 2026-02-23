#pragma once
#include <Arduino.h>
#include "SD_MMC.h"
#include "wasm3.h"

class Wasm {
public:
    static bool Init();
    static bool Run(const char* path);

private:
    static IM3Environment env;
    static IM3Runtime     runtime;

    static constexpr uint32_t RUNTIME_STACK_SIZE = 8 * 1024;   // 8 KB
    static constexpr uint32_t WASM_STACK_SIZE    = 8 * 1024;   // 8 KB
    static constexpr uint32_t WASM_HEAP_SIZE     = 6 * 1024 * 1024; // 6 MB PSRAM
};
