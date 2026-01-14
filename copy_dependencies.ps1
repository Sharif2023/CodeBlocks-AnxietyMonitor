# Script to copy required DLLs for AnxietyMonitor plugin
# Run this script from the project root directory

$CodeBlocksPluginsDir = "E:\Program Files\CodeBlocks\share\CodeBlocks\plugins"
$wxWidgetsPath = "E:\Program Files\wxWidgets\install"

Write-Host "Copying required DLLs for AnxietyMonitor plugin..." -ForegroundColor Green

# Check if plugins directory exists
if (-not (Test-Path $CodeBlocksPluginsDir)) {
    Write-Host "ERROR: Plugins directory not found: $CodeBlocksPluginsDir" -ForegroundColor Red
    exit 1
}

# Find wxWidgets DLLs
$wxDlls = @()
$searchPaths = @(
    "$wxWidgetsPath\bin",
    "$wxWidgetsPath\lib\gcc_x64_dll",
    "$wxWidgetsPath\lib\gcc_dll"
)

foreach ($path in $searchPaths) {
    if (Test-Path $path) {
        Write-Host "Checking: $path" -ForegroundColor Yellow
        $found = Get-ChildItem $path -Filter "wxbase*.dll" -ErrorAction SilentlyContinue
        if ($found) {
            Write-Host "Found wxWidgets DLLs in: $path" -ForegroundColor Green
            $wxDlls = Get-ChildItem $path -Filter "wx*.dll" -ErrorAction SilentlyContinue
            break
        }
    }
}

if ($wxDlls.Count -eq 0) {
    Write-Host "WARNING: Could not find wxWidgets DLLs. They may be needed." -ForegroundColor Yellow
    Write-Host "Please manually copy wxWidgets DLLs to: $CodeBlocksPluginsDir" -ForegroundColor Yellow
} else {
    Write-Host "Copying wxWidgets DLLs..." -ForegroundColor Green
    foreach ($dll in $wxDlls) {
        $dest = Join-Path $CodeBlocksPluginsDir $dll.Name
        Copy-Item $dll.FullName $dest -Force
        Write-Host "  Copied: $($dll.Name)" -ForegroundColor Cyan
    }
}

# Copy MinGW runtime DLLs if they exist in common locations
$mingwPaths = @(
    "C:\mingw64\bin",
    "C:\mingw32\bin",
    "C:\msys64\mingw64\bin",
    "C:\msys64\mingw32\bin"
)

$mingwDlls = @("libstdc++-6.dll", "libgcc_s_seh-1.dll", "libwinpthread-1.dll")

foreach ($mingwPath in $mingwPaths) {
    if (Test-Path $mingwPath) {
        Write-Host "Found MinGW in: $mingwPath" -ForegroundColor Green
        foreach ($dllName in $mingwDlls) {
            $src = Join-Path $mingwPath $dllName
            if (Test-Path $src) {
                $dest = Join-Path $CodeBlocksPluginsDir $dllName
                Copy-Item $src $dest -Force
                Write-Host "  Copied: $dllName" -ForegroundColor Cyan
            }
        }
        break
    }
}

Write-Host "`nDone! Please restart CodeBlocks." -ForegroundColor Green
Write-Host "Plugin location: $CodeBlocksPluginsDir" -ForegroundColor Cyan
