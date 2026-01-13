@echo off
REM ============================================================================
REM Anxiety Monitor Plugin - Installation Script for Windows
REM ============================================================================

echo.
echo ========================================
echo  Anxiety Monitor Plugin Installer
echo ========================================
echo.

REM Check if running as administrator
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo [WARNING] Not running as administrator.
    echo           You may need admin rights to install to Program Files.
    echo.
)

REM Default Code::Blocks paths
set CB_PATH_DEFAULT=C:\Program Files\CodeBlocks
set CB_PATH_X86=C:\Program Files (x86)\CodeBlocks

REM Check for Code::Blocks installation
if exist "%CB_PATH_DEFAULT%\codeblocks.exe" (
    set CB_PATH=%CB_PATH_DEFAULT%
) else if exist "%CB_PATH_X86%\codeblocks.exe" (
    set CB_PATH=%CB_PATH_X86%
) else (
    echo [ERROR] Code::Blocks not found in default locations.
    echo         Please specify the installation path:
    set /p CB_PATH="Enter Code::Blocks path: "
)

set PLUGINS_DIR=%CB_PATH%\share\codeblocks\plugins

REM Check if plugins directory exists
if not exist "%PLUGINS_DIR%" (
    echo [ERROR] Plugins directory not found: %PLUGINS_DIR%
    echo         Please check your Code::Blocks installation.
    pause
    exit /b 1
)

echo.
echo Code::Blocks found at: %CB_PATH%
echo Plugins directory: %PLUGINS_DIR%
echo.

REM Find the built plugin
set PLUGIN_FILE=
if exist "build\Release\AnxietyMonitor.dll" (
    set PLUGIN_FILE=build\Release\AnxietyMonitor.dll
) else if exist "build\Debug\AnxietyMonitor.dll" (
    set PLUGIN_FILE=build\Debug\AnxietyMonitor.dll
) else if exist "build\AnxietyMonitor.dll" (
    set PLUGIN_FILE=build\AnxietyMonitor.dll
) else if exist "AnxietyMonitor.dll" (
    set PLUGIN_FILE=AnxietyMonitor.dll
)

if "%PLUGIN_FILE%"=="" (
    echo [ERROR] Plugin file not found.
    echo         Please build the plugin first using:
    echo           cmake -B build
    echo           cmake --build build --config Release
    pause
    exit /b 1
)

echo Found plugin: %PLUGIN_FILE%
echo.
echo Installing to: %PLUGINS_DIR%
echo.

REM Copy the plugin
copy /Y "%PLUGIN_FILE%" "%PLUGINS_DIR%\AnxietyMonitor.dll"
if %errorLevel% neq 0 (
    echo [ERROR] Failed to copy plugin.
    echo         Try running as administrator.
    pause
    exit /b 1
)

echo.
echo ========================================
echo  Installation Complete!
echo ========================================
echo.
echo The Anxiety Monitor plugin has been installed.
echo.
echo Next steps:
echo 1. Restart Code::Blocks
echo 2. Go to Plugins menu to find "Anxiety Monitor"
echo 3. Click Start to begin monitoring
echo.
echo CSV files will be saved to:
echo %USERPROFILE%\.codeblocks\anxiety_monitor\sessions\
echo.

pause
