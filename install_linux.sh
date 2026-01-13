#!/bin/bash
# ============================================================================
# Anxiety Monitor Plugin - Installation Script for Linux
# ============================================================================

echo ""
echo "========================================"
echo " Anxiety Monitor Plugin Installer"
echo "========================================"
echo ""

# Default Code::Blocks paths
CB_PATHS=(
    "/usr/share/codeblocks"
    "/usr/local/share/codeblocks"
    "$HOME/.local/share/codeblocks"
)

PLUGINS_DIR=""

# Find Code::Blocks plugins directory
for path in "${CB_PATHS[@]}"; do
    if [ -d "$path/plugins" ]; then
        PLUGINS_DIR="$path/plugins"
        break
    fi
done

if [ -z "$PLUGINS_DIR" ]; then
    echo "[ERROR] Code::Blocks plugins directory not found."
    echo "        Please specify the path:"
    read -p "Enter plugins directory path: " PLUGINS_DIR
fi

if [ ! -d "$PLUGINS_DIR" ]; then
    echo "[ERROR] Directory does not exist: $PLUGINS_DIR"
    exit 1
fi

echo "Plugins directory: $PLUGINS_DIR"
echo ""

# Find the built plugin
PLUGIN_FILE=""
for file in "build/libAnxietyMonitor.so" "build/AnxietyMonitor.so" "libAnxietyMonitor.so"; do
    if [ -f "$file" ]; then
        PLUGIN_FILE="$file"
        break
    fi
done

if [ -z "$PLUGIN_FILE" ]; then
    echo "[ERROR] Plugin file not found."
    echo "        Please build the plugin first using:"
    echo "          cmake -B build"
    echo "          cmake --build build"
    exit 1
fi

echo "Found plugin: $PLUGIN_FILE"
echo ""
echo "Installing to: $PLUGINS_DIR"
echo ""

# Check if we need sudo
if [ -w "$PLUGINS_DIR" ]; then
    cp "$PLUGIN_FILE" "$PLUGINS_DIR/libAnxietyMonitor.so"
else
    echo "Need administrator privileges to install..."
    sudo cp "$PLUGIN_FILE" "$PLUGINS_DIR/libAnxietyMonitor.so"
fi

if [ $? -ne 0 ]; then
    echo "[ERROR] Failed to copy plugin."
    exit 1
fi

# Set permissions
chmod 644 "$PLUGINS_DIR/libAnxietyMonitor.so" 2>/dev/null || \
    sudo chmod 644 "$PLUGINS_DIR/libAnxietyMonitor.so"

echo ""
echo "========================================"
echo " Installation Complete!"
echo "========================================"
echo ""
echo "The Anxiety Monitor plugin has been installed."
echo ""
echo "Next steps:"
echo "1. Restart Code::Blocks"
echo "2. Go to Plugins menu to find 'Anxiety Monitor'"
echo "3. Click Start to begin monitoring"
echo ""
echo "CSV files will be saved to:"
echo "~/.codeblocks/anxiety_monitor/sessions/"
echo ""
