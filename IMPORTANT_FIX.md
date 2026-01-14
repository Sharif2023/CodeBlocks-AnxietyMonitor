# IMPORTANT: Plugin Loading Fix

## The Problem

The plugin was built with `-DWXUSINGDLL` flag, which tells the code to **dynamically load wxWidgets DLLs** at runtime. However, we're linking against **static libraries** (`.a` files), so:

1. The plugin tries to load wxWidgets DLLs when it starts
2. The DLLs don't exist (we only have static `.a` libraries)
3. CodeBlocks fails to load the plugin

## The Solution

**Static linking** - Remove `-DWXUSINGDLL` so wxWidgets code is compiled directly into the plugin DLL. This means:
- ✅ No need to copy wxWidgets DLLs
- ✅ Plugin is self-contained
- ✅ Easier distribution (only need the plugin DLL + MinGW runtime DLLs)

## What I Fixed

Updated `CMakeLists.txt` to remove `-DWXUSINGDLL` from all wxWidgets configurations. The plugin will now link wxWidgets statically.

## Next Steps

1. **Rebuild the plugin**:
   ```bash
   cmake -B build -DCODEBLOCKS_SDK_PATH="E:/Program Files/CodeBlocks"
   cmake --build build --config Release
   ```

2. **Copy the new DLL** (no wxWidgets DLLs needed!):
   ```powershell
   Copy-Item "build\AnxietyMonitor.dll" "E:\Program Files\CodeBlocks\share\CodeBlocks\plugins\" -Force
   ```

3. **Verify MinGW runtime DLLs are present** (these are still needed):
   - `libstdc++-6.dll`
   - `libgcc_s_seh-1.dll`
   - `libwinpthread-1.dll`
   
   These should already be in the plugins directory.

4. **Restart CodeBlocks**

## Why This Works

With static linking:
- All wxWidgets code is compiled into `AnxietyMonitor.dll`
- The plugin doesn't need separate wxWidgets DLLs
- Only the MinGW C++ runtime DLLs are needed (which you already have)

This is actually **better** for distribution because participants only need:
- `AnxietyMonitor.dll`
- `manifest.xml`
- MinGW runtime DLLs (if not already in system PATH)

No wxWidgets DLLs needed!
