# AnxietyMonitor Plugin Installation Guide

## Requirements

1. CodeBlocks installed at: `E:\Program Files\CodeBlocks`
2. wxWidgets DLLs must be accessible

## Installation Steps

### 1. Copy the DLL
Copy `AnxietyMonitor.dll` to:
```
E:\Program Files\CodeBlocks\share\CodeBlocks\plugins\
```

### 2. Copy wxWidgets DLLs (REQUIRED)
The plugin requires wxWidgets DLLs. Copy these files from:
```
E:\Program Files\wxWidgets\install\lib\gcc_x64_dll\
```

To one of these locations:
- **Option A**: Same directory as the plugin:
  ```
  E:\Program Files\CodeBlocks\share\CodeBlocks\plugins\
  ```
  
- **Option B**: CodeBlocks bin directory:
  ```
  E:\Program Files\CodeBlocks\
  ```

Required DLLs:
- `wxbase33u_gcc_x64.dll` (or `wxbase33u.dll`)
- `wxmsw33u_core_gcc_x64.dll` (or `wxmsw33u_core.dll`)
- `wxmsw33u_adv_gcc_x64.dll` (or `wxmsw33u_adv.dll`)

### 3. Verify manifest.xml
Ensure `manifest.xml` is in the plugins directory:
```
E:\Program Files\CodeBlocks\share\CodeBlocks\plugins\manifest.xml
```

### 4. Restart CodeBlocks
Close and restart CodeBlocks completely.

### 5. Enable the Plugin
1. Go to `Plugins` → `Manage plugins...`
2. Find "Anxiety Monitor" in the list
3. Check the box to enable it
4. Click OK

## Troubleshooting

### "DLL failed to open" Error

This usually means:
1. **Missing wxWidgets DLLs** - Copy the required DLLs (see step 2 above)
2. **Architecture mismatch** - Ensure DLLs match CodeBlocks architecture (32-bit vs 64-bit)
3. **Wrong SDK version** - The plugin is built for SDK 2.0.0

### Check Dependencies
Use Dependency Walker or similar tool to check for missing DLLs:
```
depends.exe E:\Program Files\CodeBlocks\share\CodeBlocks\plugins\AnxietyMonitor.dll
```

### Verify Plugin Architecture
CodeBlocks and the plugin must match:
- Both 32-bit, OR
- Both 64-bit

Check CodeBlocks version:
- 32-bit CodeBlocks → Need 32-bit plugin and DLLs
- 64-bit CodeBlocks → Need 64-bit plugin and DLLs

## Building the Plugin

To rebuild the plugin:

```bash
# Configure
cmake -B build -DCODEBLOCKS_SDK_PATH="E:/Program Files/CodeBlocks"

# Build
cmake --build build --config Release

# The DLL will be in: build/AnxietyMonitor.dll
```

## Notes

- The plugin uses wxWidgets 3.3 libraries
- Make sure all wxWidgets DLLs are accessible
- The plugin creates data files in: `%USERPROFILE%\.codeblocks\anxiety_monitor\sessions\`
