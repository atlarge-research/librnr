param (
    [Parameter(Mandatory)][System.IO.FileInfo]$TraceFile,
    [Parameter(Mandatory)][System.IO.FileInfo]$App,
    [Parameter(Mandatory)][System.IO.FileInfo]$OutDir
)

function Trace-Metrics([System.IO.FileInfo]$OutDir) {
    # https://xkln.net/blog/powershell-sleep-duration-accuracy-and-windows-timers/

    adb shell "cat /proc/version" >> "$OutDir\version.log"
    adb shell "cat /proc/cpuinfo" >> "$OutDir\cpuinfo.log"

    $VrJob = Start-Job -ScriptBlock { adb logcat -s VrApi >> "$OutDir\logcat_VrApi.log" }
    $HostJob = Start-Job -ScriptBlock { python "$PSScriptRoot\sample-host-metrics.py" $OutDir }

    $Freq = [System.Diagnostics.Stopwatch]::Frequency

    $Start = [System.Diagnostics.Stopwatch]::GetTimestamp()
    $i = 0

    try {
        While ($True) {
            [System.DateTime]::Now.ToString("HH:mm:ss.fff")

            if ($VrJob.State -ne "Running") {
                Write-Host "Oh no! Restarting adb logcat"
                $VrJob = Start-Job -ScriptBlock { adb logcat -s VrApi >> "$OutDir\logcat_VrApi.log" }
            }
            if ($HostJob.State -ne "Running") {
                Write-Host "Oh no! Restarting python script"
                $HostJob = Start-Job -ScriptBlock { python "$PSScriptRoot\sample-host-metrics.py" $OutDir }
            }

            adb shell "cat /proc/uptime" >> "$OutDir\uptime.log"
            adb shell "cat /proc/net/dev" >> "$OutDir\net_dev.log"
            adb shell "cat /proc/meminfo" >> "$OutDir\meminfo.log"
            adb shell "cat /proc/stat" >> "$OutDir\stat.log"
            adb shell "cat /proc/loadavg" >> "$OutDir\loadavg.log"

            adb shell "dumpsys battery" >> "$OutDir\battery.log"
            adb shell "dumpsys OVRRemoteService" >> "$OutDir\OVRRemoteService.log"
            adb shell "dumpsys CompanionService" >> "$OutDir\CompanionService.log"

            $End = [System.Diagnostics.Stopwatch]::GetTimestamp()
            Do {
                $i = $i + 1
                $Next = $Start + ($i * $Freq)
                $Sleep = $Next - $End
            } While($Sleep -lt 0)
            [System.Threading.Thread]::Sleep($Sleep * (1000.0 / $Freq))
        }
    }
    finally {
        Write-Host "Stopping VR monitor..."
        Stop-Job $VrJob
        Write-Host "Stopping host monitor..."
        Stop-Job $HostJob
    }
}

function Get-Duration([System.IO.FileInfo]$TraceFile) {
    $FirstLine = Get-Content -Head 1 $TraceFile
    $LastLine = Get-Content -Tail 1 $TraceFile
    $FirstTimestamp = [int]($FirstLine.Split(" "))[0]
    $LastTimestamp = [int]($LastLine.Split(" "))[0]
    return $LastTimestamp - $FirstTimestamp
}

$rnrDirPath = "$env:LOCALAPPDATA\librnr"
$modeFilePath = "$rnrDirPath\mode.txt"

# Set librnr to replay mode
Set-Content -Path $modeFilePath -Value "replay $TraceFile"

# Start tracing
$TraceJob = Start-Job -ScriptBlock { Trace-Metrics }

# Obtain length of trace
$Seconds = Get-Duration $TraceFile

# Start app
$Process = Start-Process -FilePath $App -PassThru

# Wait for the trace to complete
Start-Sleep -Seconds $Seconds

# Stop app after tracing
Stop-Process $Process.Id -Force

# Stop tracing
Stop-Job $TraceJob

# Plot results
Copy-Item -Path .\README.Rmd $OutDir

Write-Output "Trace complete. Open $OutDir\README.Rmd to view results."
