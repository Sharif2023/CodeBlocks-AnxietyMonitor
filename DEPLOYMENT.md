# Deployment Guide for AnxietyMonitor Plugin

## 1. Locate the Plugin Files
You need **TWO** files for the plugin to work:
1.  **DLL**: `f:\CodeBlocks-AnxietyMonitor\build\AnxietyMonitor.dll` (or `build\Release`)
2.  **ZIP**: `f:\CodeBlocks-AnxietyMonitor\build\AnxietyMonitor.zip` (I just created this)

## 2. Installation on Participant's PC

### Step 1: Browse to Plugins Directory
Navigate to the Code::Blocks execution directory.
-   **Typical Path**: `C:\Program Files\CodeBlocks\share\CodeBlocks\plugins\`
-   or `C:\Program Files (x86)\CodeBlocks\share\CodeBlocks\plugins\`

### Step 2: Copy BOTH Files
Copy **both** `AnxietyMonitor.dll` and `AnxietyMonitor.zip` into that folder.

### Step 3: Restart Code::Blocks
Close and reopen Code::Blocks.

## 3. Verify Installation
1.  Go to `Plugins` -> `Manage plugins...`
2.  You should see "Anxiety Monitor" in the list.

## 4. Troubleshooting
If it still doesn't appear:
-   **Missing Dependencies**: The DLL might need MinGW runtime libraries (`libstd c++-6.dll`, `libgcc_s_seh-1.dll`, `libwinpthread-1.dll`). Copy these from your MinGW/bin folder to the Code::Blocks folder (directory where codeblocks.exe is).
-   **Architecture**: Ensure your DLL (64-bit) matches the installed Code::Blocks (must be 64-bit).
