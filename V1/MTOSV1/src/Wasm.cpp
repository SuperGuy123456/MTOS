#include "Wasm.h"
#include "API.h"
#include "wasm3.h"

IM3Environment Wasm::env     = nullptr;
IM3Runtime     Wasm::runtime = nullptr;

// ---------------------------------------------------------
//  Initialization
// ---------------------------------------------------------
bool Wasm::Init()
{
    Serial.println("[WASM] Init");

    env = m3_NewEnvironment();
    if (!env) {
        Serial.println("[WASM] Failed to create environment");
        return false;
    }

    runtime = m3_NewRuntime(env, RUNTIME_STACK_SIZE, NULL);
    if (!runtime) {
        Serial.println("[WASM] Failed to create runtime");
        return false;
    }

    Serial.println("[WASM] Init OK");
    return true;
}

// ---------------------------------------------------------
//  Wrappers (WASM → MTOS API glue)
// ---------------------------------------------------------

// DrawText(text, x, y, fg, bg, size, nobg, centered)
// draw_text(text, x, y, fg, bg, size, nobg, centered)
m3ApiRawFunction(Wasm_DrawText)
{
    m3ApiGetArgMem(const char*, text);
    m3ApiGetArg(int32_t, x);
    m3ApiGetArg(int32_t, y);
    m3ApiGetArg(int32_t, fg);
    m3ApiGetArg(int32_t, bg);
    m3ApiGetArg(int32_t, size);
    m3ApiGetArg(int32_t, nobg);
    m3ApiGetArg(int32_t, centered);

    API::DrawText(text, x, y, fg, bg, size, nobg, centered);

    m3ApiSuccess();          // void return → fine
}

// poll_touch(outPtr)
// outPtr[0] = x
// outPtr[1] = y
// returns 1 if touched, 0 otherwise
m3ApiRawFunction(Wasm_PollTouch)
{
    m3ApiGetArgMem(int32_t*, outPtr);

    Vector2 c;
    bool touched = API::PollTouch(c);

    outPtr[0] = c.x;
    outPtr[1] = c.y;

    // i32 return value goes in first stack slot
    _sp[0] = touched ? 1 : 0;

    return m3Err_none;
}

// draw_raw(path, x, y, w, h)
// returns 1 on success, 0 on failure
m3ApiRawFunction(Wasm_DrawRaw)
{
    m3ApiGetArgMem(const char*, path);
    m3ApiGetArg(int32_t, x);
    m3ApiGetArg(int32_t, y);
    m3ApiGetArg(int32_t, w);
    m3ApiGetArg(int32_t, h);

    bool ok = API::DrawRaw(path, x, y, w, h);

    // i32 return value
    _sp[0] = ok ? 1 : 0;

    return m3Err_none;
}


// ---------------------------------------------------------
//  Run a WASM file from SD_MMC
// ---------------------------------------------------------
bool Wasm::Run(const char* path)
{
    Serial.printf("PSRAM free: %d\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
Serial.printf("PSRAM largest block: %d\n", heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));
Serial.println(psramFound());

    Serial.printf("[WASM] Loading %s\n", path);

    File f = SD_MMC.open(path, FILE_READ);
    if (!f) {
        Serial.println("[WASM] Failed to open file");
        return false;
    }

    size_t size = f.size();
    uint8_t* wasmBytes = (uint8_t*) malloc(size);
    if (!wasmBytes) {
        Serial.println("[WASM] malloc failed");
        f.close();
        return false;
    }

    f.read(wasmBytes, size);
    f.close();

    IM3Module module;
    M3Result result = m3_ParseModule(env, &module, wasmBytes, size);
    if (result) {
        Serial.printf("[WASM] Parse error: %s\n", result);
        free(wasmBytes);
        return false;
    }

    result = m3_LoadModule(runtime, module);
    if (result) {
        Serial.printf("[WASM] Load error: %s\n", result);
        free(wasmBytes);
        return false;
    }

    // -----------------------------------------------------
    //  Link MTOS API functions
    // -----------------------------------------------------
    //m3_LinkRawFunction(module, "env", "DrawText", "v(*iiiiibb)", Wasm_DrawText);
    m3_LinkRawFunction(module, "env", "DrawText",
                                      "v(*iiiiiii)",  // 1 ptr + 7 i32
                                      Wasm_DrawText);

    m3_LinkRawFunction(module, "env", "PollTouch", "i(*)",        Wasm_PollTouch);
    m3_LinkRawFunction(module, "env", "DrawRaw",  "i(*iiiii)",   Wasm_DrawRaw);

    // -----------------------------------------------------
    //  Find and call main()
    // -----------------------------------------------------
    IM3Function mainFunc;
    result = m3_FindFunction(&mainFunc, runtime, "main");
    if (result) {
        Serial.printf("[WASM] main() not found: %s\n", result);
        free(wasmBytes);
        return false;
    }

    Serial.println("[WASM] Running main()...");
    result = m3_CallV(mainFunc);

    if (result) {
        Serial.printf("[WASM] Runtime error: %s\n", result);
    } else {
        Serial.println("[WASM] main() finished OK");
    }

    free(wasmBytes);

    
    return result == m3Err_none;
}
