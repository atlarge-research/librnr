#Requires -RunAsAdministrator

$rnrDirPath = "$env:LOCALAPPDATA\librnr"
$Name = "$rnrDirPath\manifest.json"
$rnrFilePath = "$rnrDirPath\rnr.dll"
$Path = "HKLM:\SOFTWARE\Khronos\OpenXR\1\ApiLayers\Implicit"
$ApkFilePath = "com.example.batterymanager_utility.apk"

# Check that the headset is connected via ADB
$confirmation = Read-Host "Is your VR device connected via ADB? [yn]"
if ($confirmation -ne 'y') {
    Write-Host "Please connect your VR device and try again."
    exit 1
}

# Create or set registry key value to 0 to enable library
if (!(Test-Path $Path)) {
    New-Item -Path $Path -Force | Out-Null
}
Set-ItemProperty -Path $Path -Name $Name -Value 0 -Type DWord

# Create librnr directory
New-Item -ItemType Directory $rnrDirPath

# Download manifest.json
Write-Output "downloading library manifest..."
Invoke-WebRequest -Uri "https://github.com/atlarge-research/librnr/releases/latest/download/manifest.json" -OutFile $Name

# Download rnr.dll
Write-Output "downloading library..."
Invoke-WebRequest -Uri "https://github.com/atlarge-research/librnr/releases/latest/download/rnr.dll" -OutFile $rnrFilePath

Write-Output "installing battery measurement APK..."
Invoke-WebRequest -Uri "https://github.com/S2-group/batterymanager-companion/releases/latest/download/com.example.batterymanager_utility.apk" -OutFile $ApkFilePath
adb install -g $ApkFilePath
Remove-Item $ApkFilePath

Write-Output "done!"
