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

// =========================
// SCREEN + DRAWING
// =========================

m3ApiRawFunction(WASM_DrawText)
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

    m3ApiSuccess();
}

m3ApiRawFunction(WASM_DrawPixel)
{
    m3ApiGetArg(int32_t, x);
    m3ApiGetArg(int32_t, y);
    m3ApiGetArg(int32_t, color);

    bool ok = API::DrawPixel(x, y, color);
    _sp[0] = ok ? 1 : 0;

    return m3Err_none;
}

m3ApiRawFunction(WASM_ClearScreen)
{
    m3ApiGetArg(int32_t, color);

    bool ok = API::ClearScreen(color);
    _sp[0] = ok ? 1 : 0;

    return m3Err_none;
}

m3ApiRawFunction(WASM_DrawRect)
{
    m3ApiGetArg(int32_t, x);
    m3ApiGetArg(int32_t, y);
    m3ApiGetArg(int32_t, w);
    m3ApiGetArg(int32_t, h);
    m3ApiGetArg(int32_t, color);

    bool ok = API::DrawRect(x, y, w, h, color);
    _sp[0] = ok ? 1 : 0;

    return m3Err_none;
}

m3ApiRawFunction(WASM_FillRect)
{
    m3ApiGetArg(int32_t, x);
    m3ApiGetArg(int32_t, y);
    m3ApiGetArg(int32_t, w);
    m3ApiGetArg(int32_t, h);
    m3ApiGetArg(int32_t, color);

    bool ok = API::FillRect(x, y, w, h, color);
    _sp[0] = ok ? 1 : 0;

    return m3Err_none;
}

m3ApiRawFunction(WASM_DrawCircle)
{
    m3ApiGetArg(int32_t, x);
    m3ApiGetArg(int32_t, y);
    m3ApiGetArg(int32_t, r);
    m3ApiGetArg(int32_t, color);

    bool ok = API::DrawCircle(x, y, r, color);
    _sp[0] = ok ? 1 : 0;

    return m3Err_none;
}

m3ApiRawFunction(WASM_FillCircle)
{
    m3ApiGetArg(int32_t, x);
    m3ApiGetArg(int32_t, y);
    m3ApiGetArg(int32_t, r);
    m3ApiGetArg(int32_t, color);

    bool ok = API::FillCircle(x, y, r, color);
    _sp[0] = ok ? 1 : 0;

    return m3Err_none;
}

m3ApiRawFunction(WASM_DrawRaw)
{
    m3ApiGetArgMem(const char*, path);
    m3ApiGetArg(int32_t, x);
    m3ApiGetArg(int32_t, y);
    m3ApiGetArg(int32_t, w);
    m3ApiGetArg(int32_t, h);

    bool ok = API::DrawRaw(path, x, y, w, h);
    _sp[0] = ok ? 1 : 0;

    return m3Err_none;
}


// =========================
// TOUCH
// =========================

m3ApiRawFunction(WASM_PollTouch)
{
    m3ApiGetArgMem(int32_t*, outPtr);

    Vector2 c;
    bool touched = API::PollTouch(c);

    outPtr[0] = c.x;
    outPtr[1] = c.y;

    _sp[0] = touched ? 1 : 0;

    return m3Err_none;
}


// =========================
// WIFI + NETWORK
// =========================

m3ApiRawFunction(WASM_ConnectToWiFi)
{
    m3ApiGetArgMem(const char*, ssid);
    m3ApiGetArgMem(const char*, password);

    bool ok = API::ConnectToWiFi(ssid, password);
    _sp[0] = ok ? 1 : 0;

    return m3Err_none;
}

m3ApiRawFunction(WASM_DownloadFile)
{
    m3ApiGetArgMem(const char*, url);
    m3ApiGetArgMem(const char*, localPath);

    bool ok = API::DownloadFile(url, localPath);
    _sp[0] = ok ? 1 : 0;

    return m3Err_none;
}

m3ApiRawFunction(WASM_DownloadApp)
{
    m3ApiGetArgMem(const char*, name);

    bool ok = API::DownloadApp(name);
    _sp[0] = ok ? 1 : 0;

    return m3Err_none;
}

m3ApiRawFunction(WASM_CheckManifestUpToDate)
{
    bool ok = API::CheckManifestUpToDate();
    _sp[0] = ok ? 1 : 0;

    return m3Err_none;
}


// =========================
// FILESYSTEM
// =========================

m3ApiRawFunction(WASM_RecursiveDelete)
{
    m3ApiGetArgMem(const char*, path);

    bool ok = API::RecursiveDelete(path);
    _sp[0] = ok ? 1 : 0;

    return m3Err_none;
}

m3ApiRawFunction(WASM_FileExists)
{
    m3ApiGetArgMem(const char*, path);

    bool ok = API::FileExists(path);
    _sp[0] = ok ? 1 : 0;

    return m3Err_none;
}

m3ApiRawFunction(WASM_ReadFile)
{
    m3ApiGetArgMem(const char*, path);
    m3ApiGetArgMem(char*, buffer);
    m3ApiGetArg(int32_t, bufferSize);

    bool ok = API::ReadFile(path, buffer, bufferSize);
    _sp[0] = ok ? 1 : 0;

    return m3Err_none;
}

m3ApiRawFunction(WASM_WriteFile)
{
    m3ApiGetArgMem(const char*, path);
    m3ApiGetArgMem(const char*, data);

    bool ok = API::WriteFile(path, data);
    _sp[0] = ok ? 1 : 0;

    return m3Err_none;
}

m3ApiRawFunction(WASM_AppendToFile)
{
    m3ApiGetArgMem(const char*, path);
    m3ApiGetArgMem(const char*, data);

    bool ok = API::AppendToFile(path, data);
    _sp[0] = ok ? 1 : 0;

    return m3Err_none;
}

m3ApiRawFunction(WASM_ListDirectory)
{
    m3ApiGetArgMem(const char*, path);

    bool ok = API::ListDirectory(path);
    _sp[0] = ok ? 1 : 0;

    return m3Err_none;
}

m3ApiRawFunction(WASM_CreateDirectory)
{
    m3ApiGetArgMem(const char*, path);

    bool ok = API::CreateDirectory(path);
    _sp[0] = ok ? 1 : 0;

    return m3Err_none;
}


// =========================
// TIME + DELAY
// =========================

m3ApiRawFunction(WASM_GetTime)
{
    uint32_t t = API::GetTime();
    _sp[0] = t;

    return m3Err_none;
}

m3ApiRawFunction(WASM_Delay)
{
    m3ApiGetArg(int32_t, ms);

    API::Delay(ms);
    m3ApiSuccess();
}


// =========================
// RANDOM
// =========================

m3ApiRawFunction(WASM_Random)
{
    m3ApiGetArg(int32_t, min);
    m3ApiGetArg(int32_t, max);

    int r = API::Random(min, max);
    _sp[0] = r;

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
    // =========================
    // SCREEN + DRAWING
    // =========================

    m3_LinkRawFunction(module, "env", "DrawText",
                    "v(*iiiiiii)",   // text*, x, y, fg, bg, size, nobg, centered
                    WASM_DrawText);

    m3_LinkRawFunction(module, "env", "DrawPixel",
                    "i(iii)",        // x, y, color
                    WASM_DrawPixel);

    m3_LinkRawFunction(module, "env", "ClearScreen",
                    "i(i)",          // color
                    WASM_ClearScreen);

    m3_LinkRawFunction(module, "env", "DrawRect",
                    "i(iiiii)",      // x, y, w, h, color
                    WASM_DrawRect);

    m3_LinkRawFunction(module, "env", "FillRect",
                    "i(iiiii)",      // x, y, w, h, color
                    WASM_FillRect);

    m3_LinkRawFunction(module, "env", "DrawCircle",
                    "i(iiii)",       // x, y, r, color
                    WASM_DrawCircle);

    m3_LinkRawFunction(module, "env", "FillCircle",
                    "i(iiii)",       // x, y, r, color
                    WASM_FillCircle);

    m3_LinkRawFunction(module, "env", "DrawRaw",
                    "i(*iiiii)",     // path*, x, y, w, h
                    WASM_DrawRaw);


    // =========================
    // TOUCH
    // =========================

    m3_LinkRawFunction(module, "env", "PollTouch",
                    "i(*)",          // outPtr*
                    WASM_PollTouch);


    // =========================
    // WIFI + NETWORK
    // =========================

    m3_LinkRawFunction(module, "env", "ConnectToWiFi",
                    "i(**)",         // ssid*, password*
                    WASM_ConnectToWiFi);

    m3_LinkRawFunction(module, "env", "DownloadFile",
                    "i(**)",         // url*, localPath*
                    WASM_DownloadFile);

    m3_LinkRawFunction(module, "env", "DownloadApp",
                    "i(*)",          // name*
                    WASM_DownloadApp);

    m3_LinkRawFunction(module, "env", "CheckManifestUpToDate",
                    "i()",           // no args
                    WASM_CheckManifestUpToDate);


    // =========================
    // FILESYSTEM
    // =========================

    m3_LinkRawFunction(module, "env", "RecursiveDelete",
                    "i(*)",          // path*
                    WASM_RecursiveDelete);

    m3_LinkRawFunction(module, "env", "FileExists",
                    "i(*)",          // path*
                    WASM_FileExists);

    m3_LinkRawFunction(module, "env", "ReadFile",
                    "i(* * i)",      // path*, buffer*, bufferSize
                    WASM_ReadFile);

    m3_LinkRawFunction(module, "env", "WriteFile",
                    "i(**)",         // path*, data*
                    WASM_WriteFile);

    m3_LinkRawFunction(module, "env", "AppendToFile",
                    "i(**)",         // path*, data*
                    WASM_AppendToFile);

    m3_LinkRawFunction(module, "env", "ListDirectory",
                    "i(*)",          // path*
                    WASM_ListDirectory);

    m3_LinkRawFunction(module, "env", "CreateDirectory",
                    "i(*)",          // path*
                    WASM_CreateDirectory);


    // =========================
    // TIME + DELAY
    // =========================

    m3_LinkRawFunction(module, "env", "GetTime",
                    "i()",           // no args
                    WASM_GetTime);

    m3_LinkRawFunction(module, "env", "Delay",
                    "v(i)",          // ms
                    WASM_Delay);


    // =========================
    // RANDOM
    // =========================

    m3_LinkRawFunction(module, "env", "Random",
                    "i(ii)",         // min, max
                    WASM_Random);


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
