if (!(Test-Path "./release")) {
    Write-Host "Creating release folder"
    New-Item -ItemType Directory -Path "./release"
}

$releasePath = Join-Path (Split-Path -Parent $PSCommandPath) "release"

# This assumes tcc is in the path.
Push-Location "src"
$tccOutput = tcc -o (Join-Path $releasePath "arcane.exe") *.c -lws2_32 2>&1
if ($LASTEXITCODE -eq 0) {
    Write-Host "Build succeeded" -ForegroundColor Green
} else {
    Write-Host "Build failed (exit code: $LASTEXITCODE)" -ForegroundColor Red
}
if ($tccOutput) {
    Write-Host $tccOutput
}
Pop-Location