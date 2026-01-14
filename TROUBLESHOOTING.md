# Troubleshooting "DLL failed to open" Error

## Common Causes

### 1. Missing wxWidgets DLLs (MOST COMMON)

The plugin requires wxWidgets DLLs to be accessible. CodeBlocks 25.03 uses wxWidgets 3.3.

**Required DLLs:**
- `wxbase33u_gcc_x64.dll` (or `wxbase33u.dll`)
- `wxmsw33u_core_gcc_x64.dll` (or `wxmsw33u_core.dll`)  
- `wxmsw33u_adv_gcc_x64.dll` (or `wxmsw33u_adv.dll`)

**Where to find them:**
- `E:\Program Files\wxWidgets\install\lib\gcc_x64_dll\`
- Or `E:\Program Files\wxWidgets\install\bin\`

**Where to copy them:**
Copy ALL wxWidgets DLLs to:
```
E:\Program Files\CodeBlocks\share\CodeBlocks\plugins\
```

**Quick fix script:**
```powershell
# Run from PowerShell as Administrator
$source = "E:\Program Files\wxWidgets\install\lib\gcc_x64_dll"
$dest = "E:\Program Files\CodeBlocks\share\CodeBlocks\plugins"
Copy-Item "$source\wx*.dll" $dest -Force
```

### 2. Architecture Mismatch

Ensure all DLLs match:
- **64-bit CodeBlocks** → Need 64-bit plugin and 64-bit DLLs
- **32-bit CodeBlocks** → Need 32-bit plugin and 32-bit DLLs

Check CodeBlocks architecture:
```powershell
# Check if CodeBlocks is 64-bit
[System.Reflection.Assembly]::LoadFile("E:\Program Files\CodeBlocks\codeblocks.exe").ImageRuntimeVersion
```

### 3. Missing MinGW Runtime DLLs

You've already copied these, but verify they're in the right place:
- `libstdc++-6.dll`
- `libgcc_s_seh-1.dll`  
- `libwinpthread-1.dll`

**Location:** Same directory as `AnxietyMonitor.dll`

### 4. Verify Exported Functions

The DLL must export:
- `GetPlugin()` - returns `cbPlugin*`
- `GetSDKVersion()` - returns SDK version (20000 for SDK 2.0.0)

**Check exports:**
```powershell
# Using objdump (if MinGW is in PATH)
objdump -p "E:\Program Files\CodeBlocks\share\CodeBlocks\plugins\AnxietyMonitor.dll" | Select-String "GetPlugin|GetSDKVersion"
```

### 5. Check CodeBlocks Logs

CodeBlocks logs errors to:
```
%APPDATA%\CodeBlocks\default.conf
```

Or check the CodeBlocks console output when starting.

## Step-by-Step Fix

1. **Rebuild the plugin** with the latest code:
   ```bash
   cmake -B build -DCODEBLOCKS_SDK_PATH="E:/Program Files/CodeBlocks"
   cmake --build build --config Release
   ```

2. **Copy wxWidgets DLLs** (CRITICAL):
   ```powershell
   $wxLibs = "E:\Program Files\wxWidgets\install\lib\gcc_x64_dll"
   $plugins = "E:\Program Files\CodeBlocks\share\CodeBlocks\plugins"
   Copy-Item "$wxLibs\wx*.dll" $plugins -Force
   ```

3. **Copy the new DLL**:
   ```powershell
   Copy-Item "build\AnxietyMonitor.dll" "E:\Program Files\CodeBlocks\share\CodeBlocks\plugins\" -Force
   ```

4. **Verify manifest.xml exists**:
   ```powershell
   Test-Path "E:\Program Files\CodeBlocks\share\CodeBlocks\plugins\manifest.xml"
   ```

5. **Restart CodeBlocks completely** (close all windows)

6. **Check Plugin Manager**:
   - Go to `Plugins` → `Manage plugins...`
   - Look for "Anxiety Monitor"
   - If it appears but is unchecked, enable it

## Still Not Working?

1. **Check Windows Event Viewer**:
   - Open Event Viewer
   - Look for application errors related to CodeBlocks

2. **Use Dependency Walker**:
   - Download Dependency Walker
   - Open `AnxietyMonitor.dll`
   - Look for missing DLLs (marked in red)

3. **Try building with CODEBLOCKS_SDK_INCLUDED**:
   - This requires full CodeBlocks SDK headers
   - May provide better compatibility

4. **Check CodeBlocks version compatibility**:
   - Your CodeBlocks: 25.03
   - Plugin SDK version: 2.0.0
   - These should be compatible

## Quick Test

Create a simple test to verify DLL can load:
```powershell
# Test if DLL can be loaded
Add-Type -TypeDefinition @"
using System;
using System.Runtime.InteropServices;
public class Test {
    [DllImport("E:\\Program Files\\CodeBlocks\\share\\CodeBlocks\\plugins\\AnxietyMonitor.dll")]
    public static extern IntPtr GetPlugin();
    
    [DllImport("E:\\Program Files\\CodeBlocks\\share\\CodeBlocks\\plugins\\AnxietyMonitor.dll")]
    public static extern int GetSDKVersion();
}
"@
try {
    $version = [Test]::GetSDKVersion()
    Write-Host "DLL loaded successfully! SDK Version: $version"
} catch {
    Write-Host "Error loading DLL: $_"
}
```
