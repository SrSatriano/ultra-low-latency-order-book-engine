# Build order book engine (Windows)
$ErrorActionPreference = "Stop"
$root = Split-Path $PSScriptRoot -Parent
Set-Location $root

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    Write-Error "Instale CMake e adicione ao PATH"
}

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

$test = @(
    "build\Release\orderbook_test.exe",
    "build\orderbook_test.exe"
) | Where-Object { Test-Path $_ } | Select-Object -First 1

if ($test) { & $test }

$bench = @(
    "build\Release\bench_pricing.exe",
    "build\bench_pricing.exe"
) | Where-Object { Test-Path $_ } | Select-Object -First 1

if ($bench) { & $bench }
