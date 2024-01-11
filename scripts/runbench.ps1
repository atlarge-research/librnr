param (
    [System.IO.FileInfo]$TraceFile = "trace.txt",
    [Parameter(Mandatory)][System.IO.FileInfo]$OutDir,
    [System.IO.FileInfo]$App,
    [System.String]$Class,
    [System.String]$Activity,
    [ValidateRange("Positive")][int]$Duration,
    [ValidateNotNullOrEmpty()][ValidateSet('record', 'replay')][System.String]$Mode = "replay",
    [switch]$NoHostTrace,
    [switch]$S2Battery
)

$Autodriver = $PSBoundParameters.ContainsKey('Class') -and $PSBoundParameters.ContainsKey('Activity') -and $PSBoundParameters.ContainsKey('Duration')

$functions = {
    function Trace-Metrics([string]$OutDir, [string]$PSScriptRoot) {
        # https://xkln.net/blog/powershell-sleep-duration-accuracy-and-windows-timers/

        # clear the log
        adb logcat -c
        # set the log to max size
        adb logcat -G 16M

        # start the battery measurement app
        if ($S2Battery) {
            adb shell am start-foreground-service -n "com.example.batterymanager_utility/com.example.batterymanager_utility.DataCollectionService" --ei sampleRate 1000 --es "dataFields" "BATTERY_PROPERTY_CURRENT_NOW,EXTRA_VOLTAGE" --ez toCSV False
        }

        # log the device hardware
        adb shell "cat /proc/version" >> "$OutDir\version.log"
        adb shell "cat /proc/cpuinfo" >> "$OutDir\cpuinfo.log"

        # start logging metrics from the gaming PC
        if (-not($NoHostTrace)) {
            $HostJob = Start-Job -ScriptBlock { python "$using:PSScriptRoot\sample-host-metrics.py" $using:OutDir }
        }

        $Freq = [System.Diagnostics.Stopwatch]::Frequency
        $Start = [System.Diagnostics.Stopwatch]::GetTimestamp()
        $i = 0

        try {
            # pull metrics every second
            While ($True) {
                [System.DateTime]::Now.ToString("HH:mm:ss.fff")

                # check if process that logs metrics from gaming PC is still running
                # if not, restart
                if ((-not($NoHostTrace)) -and ($HostJob.State -ne "Running")) {
                    Write-Host "Oh no! Restarting python script"
                    $HostJob = Start-Job -ScriptBlock { python "$using:PSScriptRoot\sample-host-metrics.py" $using:OutDir }
                }

                # get metrics from the VR device
                adb shell "cat /proc/uptime" >> "$OutDir\uptime.log"
                adb shell "cat /proc/net/dev" >> "$OutDir\net_dev.log"
                adb shell "cat /proc/meminfo" >> "$OutDir\meminfo.log"
                adb shell "cat /proc/stat" >> "$OutDir\stat.log"
                adb shell "cat /proc/loadavg" >> "$OutDir\loadavg.log"

                adb shell "dumpsys battery" >> "$OutDir\battery.log"
                adb shell "dumpsys OVRRemoteService" >> "$OutDir\OVRRemoteService.log"
                adb shell "dumpsys CompanionService" >> "$OutDir\CompanionService.log"

                # calculate how long to sleep to maintain sample frequency of 1Hz
                $End = [System.Diagnostics.Stopwatch]::GetTimestamp()
                Do {
                    $i = $i + 1
                    $Next = $Start + ($i * $Freq)
                    $Sleep = $Next - $End
                } While ($Sleep -lt 0)
                [System.Threading.Thread]::Sleep($Sleep * (1000.0 / $Freq))
            }
        }
        finally {
            # get logcat logs from trace
            adb logcat -d >> "$OutDir\logcat.log"

            # get only VrApi logs
            adb logcat -d -s VrApi >> "$OutDir\logcat_VrApi.log"

            # stop process collecting metrics from gaming PC
            Write-Host "Stopping host monitor..."
            Stop-Job $HostJob

            if ($S2Battery) {
                Write-Host "Stopping battery measurements..."
                # stop app collecting battery measurements
                adb shell am stopservice com.example.batterymanager_utility/com.example.batterymanager_utility.DataCollectionService
                # get the battery measurements from the VR device
                adb logcat -d | Select-String "BatteryMgr:DataCollectionService" >> "$OutDir\batterymanager-companion.log"
            }
        }
    }
}

# calculate the duration of an OpenXR trace.txt
function Get-Duration([System.IO.FileInfo]$TraceFile) {
    $FirstLine = Get-Content -Head 1 $TraceFile
    $LastLine = Get-Content -Tail 1 $TraceFile
    $FirstTimestamp = [long]($FirstLine.Split(" "))[0]
    $LastTimestamp = [long]($LastLine.Split(" "))[0]
    # Calculate duration, convert nanoseconds to seconds
    return ($LastTimestamp - $FirstTimestamp) / 1000000000
}

try {
    $rnrDirPath = "$env:LOCALAPPDATA\librnr"
    $modeFilePath = "$rnrDirPath\config.txt"
    $VrStorage = adb shell 'echo $EXTERNAL_STORAGE'

    # Create the output directory, if needed
    New-Item $OutDir -ItemType Directory -Force | Out-Null

    # Set librnr to configured mode
    Set-Content -Path $modeFilePath -Value "$Mode $TraceFile"

    # Start tracing
    $TraceJob = Start-Job -InitializationScript $functions -ScriptBlock { Trace-Metrics $using:OutDir $using:PSScriptRoot }

    if ($Autodriver) {
        adb shell setprop debug.oculus.vrapilayers AutoDriver
        adb shell setprop debug.oculus.autoDriverApp $Class
        if ($Mode -eq "replay") {
            adb push $TraceFile "$VrStorage/Android/data/$Class/AutoDriver/default.autodriver" > nul
            adb shell setprop debug.oculus.autoDriverMode Playback
            adb shell setprop debug.oculus.autoDriverPlaybackHeadMode HeadLocked
        }
        else {
            adb shell setprop debug.oculus.autoDriverMode Record
        }
        adb shell am start -S $Class/$Activity  > nul 2>&1

        Start-Sleep -Seconds $Duration
    }
    else {
        if ( $PSBoundParameters.ContainsKey('App')) {
            # Start app
            $Process = Start-Process -FilePath $App -PassThru
        }

        if ($Mode -eq "replay") {
            # Obtain length of trace
            $Seconds = (Get-Duration $TraceFile) + 5
            # Wait for the trace to complete
            Write-Output "Will sleep for duration of trace: $Seconds seconds"
            Start-Sleep -Seconds $Seconds
        }
        else {
            # Recording, sleep until the user stops the script with an interrupt
            while ($True) {
                Start-Sleep 5
            }
        }
    }
}
finally {
    if ($Autodriver) {
        adb shell am broadcast -a com.oculus.vrapilayers.AutoDriver.SHUTDOWN
        adb shell am force-stop $Class

        if ($Mode -eq "record") {
            adb pull "$VrStorage/Android/data/$Class/AutoDriver/default.autodriver" $TraceFile  > nul
        }
    }
    elseif ($PSBoundParameters.ContainsKey('App')) {
        # Stop app after tracing
        Stop-Process $Process.Id -Force -ErrorAction SilentlyContinue
    }

    # Stop tracing
    Stop-Job $TraceJob

    # Plot results
    Copy-Item -Path .\README.Rmd $OutDir

    Write-Output "Done $( $Mode )ing trace. Open $OutDir\README.Rmd to view results."
}
