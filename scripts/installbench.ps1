#Requires -RunAsAdministrator

$rnrDirPath = "$env:LOCALAPPDATA\librnr"
$Name = "$rnrDirPath\manifest.json"
$rnrFilePath = "$rnrDirPath\rnr.dll"
$Path = "HKLM:\SOFTWARE\Khronos\OpenXR\1\ApiLayers\Implicit"

# Create or set registry key value to 0 to enable library
Set-ItemProperty -Path $Path -Name $Name -Value 0 -Type DWord

# Create librnr directory
New-Item -ItemType Directory $rnrDirPath

# Download manifest.json
Write-Output "downloading library manifest..."
Invoke-WebRequest -Uri "https://github.com/atlarge-research/librnr/releases/latest/download/manifest.json" -OutFile $Name

# Download rnr.dll
Write-Output "downloading library..."
Invoke-WebRequest -Uri "https://github.com/atlarge-research/librnr/releases/latest/download/rnr.dll" -OutFile $rnrFilePath

Write-Output "done!"
