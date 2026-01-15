# AnxietyMonitor Plugin Installation Guide

## Simplified Installation (Recommended)

The plugin is packaged as a `.cbplugin` file (which contains the required inner zip structure).

1.  **Locate the File**:
    Find `AnxietyMonitor.cbplugin` in your project folder (e.g., `f:\CodeBlocks-AnxietyMonitor\`).

2.  **Install via Code::Blocks**:
    - Open Code::Blocks 25.03.
    - Go to **Plugins** -> **Manage plugins...**
    - Click **Install new**.
    - Browse to and select `AnxietyMonitor.cbplugin`.
    - Follow the prompts to install.

## Manual Installation

If the automatic installation fails:

1.  **Extract**:
    Unzip `AnxietyMonitor.cbplugin` (it is just a zip file). You should see:
    - `AnxietyMonitor.dll`
    - `AnxietyMonitor.zip` (Do NOT unzip this!)

2.  **Copy Files**:
    Copy BOTH files to your Code::Blocks plugins directory:
    - `AnxietyMonitor.dll`
    - `AnxietyMonitor.zip`
    
    Destination: `C:\Program Files\CodeBlocks\share\CodeBlocks\plugins\`

3.  **Restart**:
    Restart Code::Blocks.

## Verification

1.  Start Code::Blocks.
2.  Look for the **Anxiety Monitor** toolbar or menu entries.
3.  If not visible, check **Plugins** -> **Manage plugins...** to ensure it is enabled.
