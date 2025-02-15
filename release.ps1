# Build the release executable which can be used to run script files.

if (!(Test-Path "./release")) {
    Write-Host "Creating release folder"
    New-Item -ItemType Directory -Path "./release"
}

$releasePath = Join-Path (Split-Path -Parent $PSCommandPath) "release"

# This assumes tcc is in the path.
Push-Location "src"
$tccOutput = tcc -o (Join-Path $releasePath "arcane.exe") *.c -lws2_32 2>&1
if ($LASTEXITCODE -eq 0) {
    Write-Host "EXE Build succeeded" -ForegroundColor Green
} else {
    Write-Host "EXE Build failed (exit code: $LASTEXITCODE)" -ForegroundColor Red
}
if ($tccOutput) {
    Write-Host $tccOutput
}
Pop-Location

# Build the release DLL via "tcc -shared", excluding main.c.
Push-Location "src"
$cFiles = Get-ChildItem -Filter *.c | Where-Object { $_.Name -ne "main.c" } | ForEach-Object { $_.Name }
$tccOutputDll = tcc -shared -o (Join-Path $releasePath "arcane.dll") $cFiles -lws2_32 2>&1
if ($LASTEXITCODE -eq 0) {
    Write-Host "DLL build succeeded" -ForegroundColor Green
} else {
    Write-Host "DLL build failed (exit code: $LASTEXITCODE)" -ForegroundColor Red
}
if ($tccOutputDll) {
    Write-Host $tccOutputDll
}
Pop-Location

# Build an amalgamation file.
# Build the amalgamation C file.
$amalgamationFile = Join-Path $releasePath "arcane.c"
$filesToCombine = @("arcane.h", "functions.c", "arcane.c")

# Create or clear the amalgamation file.
Set-Content -Path $amalgamationFile -Value ""

foreach ($file in $filesToCombine) {
    $filePath = Join-Path "src" $file
    if (Test-Path $filePath) {
        Get-Content $filePath | Where-Object { $_ -notmatch '#include\s+["'']arcane\.h["'']' } | Add-Content -Path $amalgamationFile
    } else {
        Write-Host "File $file not found in src folder." -ForegroundColor Yellow
    }
}

Write-Host "Amalgamation file created at $amalgamationFile" -ForegroundColor Green