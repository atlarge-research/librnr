param (
    [Parameter(Mandatory, HelpMessage="Path to the clumsy.exe executable")]
    [string]$ClumsyPath,

    [Parameter()]
    [int]$Delay,    # How long to delay packets by (measured in milisec)

    [Parameter()]
    [int]$BandwidthKBps, # Bandwidth cap for BOTH upload AND download (0 - 99999 KiloBYTES per second)

    [Parameter()]
    [float]$DropChance, # Chance of a packet being dropped

    [Parameter()]
    [bool]$AffectUpload, # Whether the effects apply to outgoing packets. Bandwidth cap is not affected by this!!!
    
    [Parameter()]
    [bool]$AffectDownload # Whether the effects apply to incoming packets. Bandwidth cap is not affected by this!!!
)

# Sanity check that command would have AN effect on SOMETHING at least.
if ((!$AffectDownload -and !$AffectUpload) -and ($Delay -or $DropChance)) {
    echo ">>> [Error] Both AffectUpload and AffectDownload are False. Command would have no effect. Exiting."
    exit
}

if ($AffectDownload -and $AffectUpload) {
    $FilterArg = '--filter "outbound or inbound"'
}
elseif ($AffectDownload) {
    $FilterArg = '--filter "inbound"'
}
elseif ($AffectUpload) {
    $FilterArg = '--filter "outbound"'
}

# Set delay argument
$DelayArg = "--lag off --lag-outbound off --lag-inbound off"
if ($Delay -and ($AffectDownload -or $AffectUpload)) {
    $DelayArg = "--lag on --lag-time $Delay"

    if ($AffectUpload) {
        $DelayArg = "$DelayArg --lag-outbound on"
    }
    else {
        $DelayArg = "$DelayArg --lag-outbound off"
    }

    if ($AffectDownload) {
        $DelayArg = "$DelayArg --lag-inbound on"
    }
    else {
        $DelayArg = "$DelayArg --lag-inbound off"
    }
}
echo "Final delay is: $DelayArg"

# Set Bandwidth limit argument
$BandwidthArg = "--bandwidth off --bandwidth-outbound off --bandwidth-inbound off"
if ($BandwidthKBps -and ($AffectDownload -or $AffectUpload)) {
    $BandwidthArg = "--bandwidth on --bandwidth-bandwidth $BandwidthKBps"
    echo "NOTE: Bandwidth cannot be selectively applied to download or upload through AffectDownload and AffectUpload."
}
echo "Final bandwidth is: $BandwidthArg"

# Set drop chance argument
$DropChanceArg = "--drop off --drop-outbound off --drop-inbound off"
if ($DropChance -and ($AffectDownload -or $AffectUpload)) {
    $DropChanceArg = "--drop on"

    if (($DropChance -lt 0) -or ($DropChance -gt 1)) {
        echo ">>> [Error] Invalid value for DropChance. Must be between 0 and 1."
        exit
    }
    else {
        $DropChanceAdjusted = [math]::Round($DropChance * 100, 2)
        $DropChanceArg = "$DropChanceArg --drop-chance $DropChanceAdjusted"
    }

    if ($AffectUpload) {
        $DropChanceArg = "$DropChanceArg --drop-outbound on"
    }
    else {
        $DropChanceArg = "$DropChanceArg --drop-outbound off"
    }

    if ($AffectDownload) {
        $DropChanceArg = "$DropChanceArg --drop-inbound on"
    }
    else {
        $DropChanceArg = "$DropChanceArg --drop-inbound off"
    }
} 
echo "Final drop is: $DropChanceArg"

$ArgList = "$BandwidthArg $DelayArg $DropChanceArg $FilterArg"
echo "Final arg list is: $ArgList"

try {
    try {
        $ClumsyPathFull = [System.IO.Path]::GetFullPath($ClumsyPath)
    }
    catch {
        $ClumsyPathFull = $ClumsyPath
    }
    
    echo "Clumsy is at: $ClumsyPathFull"
    Start-Process -FilePath $ClumsyPathFull -Verb RunAs -ArgumentList $ArgList
}
catch {
    echo ">>> [Error] Error occurred during clumsy startup. Closing."
    .\stop-clumsy.ps1
}