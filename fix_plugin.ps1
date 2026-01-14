# Fix AnxietyMonitor Plugin Loading Issues
# Run this script as Administrator

Write-Host "=== AnxietyMonitor Plugin Fix Script ===" -ForegroundColor Green
Write-Host ""

$pluginsDir = "E:\Program Files\CodeBlocks\share\CodeBlocks\plugins"
$wxWidgetsPath = "E:\Program Files\wxWidgets\install"

# Step 1: Check wxWidgets DLLs (NOTE: With static linking, DLLs are NOT needed!)
Write-Host "Step 1: Checking wxWidgets configuration..." -ForegroundColor Yellow
Write-Host "  NOTE: Plugin uses STATIC linking - wxWidgets DLLs are NOT required!" -ForegroundColor Cyan
Write-Host "  The plugin DLL contains all wxWidgets code internally." -ForegroundColor Cyan
Write-Host ""

# Step 2: Verify MinGW runtime DLLs
Write-Host ""
Write-Host "Step 3: Verifying MinGW runtime DLLs..." -ForegroundColor Yellow
$requiredMingw = @("libstdc++-6.dll", "libgcc_s_seh-1.dll", "libwinpthread-1.dll")
$missingMingw = @()

foreach ($dll in $requiredMingw) {
    $path = Join-Path $pluginsDir $dll
    if (Test-Path $path) {
        Write-Host "  Found: $dll" -ForegroundColor Green
    } else {
        Write-Host "  Missing: $dll" -ForegroundColor Yellow
        $missingMingw += $dll
    }
}

if ($missingMingw.Count -gt 0) {
    Write-Host ""
    Write-Host "  WARNING: Some MinGW DLLs are missing. They should be in:" -ForegroundColor Yellow
    Write-Host "  $pluginsDir" -ForegroundColor Cyan
}

# Step 3: Verify plugin DLL exists
Write-Host ""
Write-Host "Step 4: Verifying plugin DLL..." -ForegroundColor Yellow
$pluginDll = Join-Path $pluginsDir "AnxietyMonitor.dll"
if (Test-Path $pluginDll) {
    Write-Host "  Found: AnxietyMonitor.dll" -ForegroundColor Green
    $dllInfo = Get-Item $pluginDll
    Write-Host "  Size: $([math]::Round($dllInfo.Length/1KB, 2)) KB" -ForegroundColor Cyan
    Write-Host "  Modified: $($dllInfo.LastWriteTime)" -ForegroundColor Cyan
} else {
    Write-Host "  ERROR: AnxietyMonitor.dll not found!" -ForegroundColor Red
    Write-Host "  Expected location: $pluginDll" -ForegroundColor Yellow
}

# Step 4: Verify manifest.xml
Write-Host ""
Write-Host "Step 5: Verifying manifest.xml..." -ForegroundColor Yellow
$manifest = Join-Path $pluginsDir "manifest.xml"
if (Test-Path $manifest) {
    Write-Host "  Found: manifest.xml" -ForegroundColor Green
} else {
    Write-Host "  WARNING: manifest.xml not found!" -ForegroundColor Yellow
    Write-Host "  Copying from project root..." -ForegroundColor Cyan
    if (Test-Path "manifest.xml") {
        Copy-Item "manifest.xml" $manifest -Force
        Write-Host "  Copied manifest.xml" -ForegroundColor Green
    } else {
        Write-Host "  ERROR: manifest.xml not found in project root!" -ForegroundColor Red
    }
}

# Summary
Write-Host ""
Write-Host "=== Summary ===" -ForegroundColor Green
Write-Host "Plugin directory: $pluginsDir" -ForegroundColor Cyan
Write-Host "wxWidgets: Static linking (no DLLs needed)" -ForegroundColor Cyan
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Yellow
Write-Host "1. Close CodeBlocks completely" -ForegroundColor White
Write-Host "2. Restart CodeBlocks" -ForegroundColor White
Write-Host "3. Go to Plugins -> Manage plugins..." -ForegroundColor White
Write-Host "4. Look for 'Anxiety Monitor' in the list" -ForegroundColor White
Write-Host ""
Write-Host "If it still doesn't work, check TROUBLESHOOTING.md" -ForegroundColor Yellow
