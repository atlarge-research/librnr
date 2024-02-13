param (
    [Parameter(Mandatory)][System.IO.FileInfo]$TraceFile,
    [Parameter(Mandatory)][System.IO.FileInfo]$OutDir,
    [System.IO.FileInfo]$App,
    [System.String]$SteamAppID,
    [System.String]$SteamAppExe,
    [System.Int16]$AppStartupTime, # in seconds
    [ValidateNotNullOrEmpty()][ValidateSet('record', 'replay')][System.String]$Mode = "replay",
    [ValidateNotNullOrEmpty()][System.String]$BMSampleRate = "1000",  # in milliseconds, how often to sample BatteryManager
    [ValidateNotNullOrEmpty()][System.String]$BMMetrics = "BATTERY_PROPERTY_CURRENT_NOW,EXTRA_VOLTAGE", # comma-separated list of BatteryManager metrics to sample, see https://developer.android.com/reference/android/os/BatteryManager
    [ValidateNotNullOrEmpty()][System.String]$OVRGPUMetrics = "1,2,3,4"  # comma-separated list of GPU metrics to sample, see https://developer.oculus.com/documentation/native/android/mobile-gpu-timers/https://developer.oculus.com/documentation/unity/ts-ovrgpuprofiler/
)

$functions = {
    function Trace-Metrics([string]$OutDir, [string]$PSScriptRoot)
    {
        "Trace-Metrics: OutDir: $OutDir, PSScriptRoot: $PSScriptRoot" | Out-Host
        # https://xkln.net/blog/powershell-sleep-duration-accuracy-and-windows-timers/
        Write-Host "Starting VR monitor..."
        adb shell "cat /proc/version" >> "$OutDir\version.log"
        adb shell "cat /proc/cpuinfo" >> "$OutDir\cpuinfo.log"

        $VrJob = Start-Job -ScriptBlock { adb logcat -s VrApi >> "$using:OutDir\logcat_VrApi.log" }
        $HostJob = Start-Job -ScriptBlock { C:/Users/damla/AppData/Local/Microsoft/WindowsApps/python3.11.exe "$using:PSScriptRoot\sample-host-metrics.py" $using:OutDir }
        $OVRGPUJob = Start-Job -ScriptBlock { adb shell ovrgpuprofiler -r"$OVRGPUMetrics" | ForEach-Object{"{0} - {1}" -f (Get-Date),$_} >> "$using:OutDir\ovr_gpu_metrics.log" }

        $Freq = [System.Diagnostics.Stopwatch]::Frequency

        $Start = [System.Diagnostics.Stopwatch]::GetTimestamp()
        $i = 0

        try
        {
            While ($True)
            {
                [System.DateTime]::Now.ToString("HH:mm:ss.fff")

                if ($VrJob.State -ne "Running")
                {
                    Write-Host "Oh no! Restarting adb logcat"
                    $VrJob = Start-Job -ScriptBlock { adb logcat -s VrApi >> "$using:OutDir\logcat_VrApi.log" }
                }
                if ($HostJob.State -ne "Running")
                {
                    Write-Host "Oh no! Restarting python script"
                    $HostJob = Start-Job -ScriptBlock { C:/Users/damla/AppData/Local/Microsoft/WindowsApps/python3.11.exe "$using:PSScriptRoot\sample-host-metrics.py" $using:OutDir }
                }
                if ($OVRGPUJob.State -ne "Running")
                {
                    Write-Host "Oh no! Restarting ovrgpuprofiler"
                    $OVRGPUJob = Start-Job -ScriptBlock { adb shell ovrgpuprofiler -r"$OVRGPUMetrics" | ForEach-Object{"{0} - {1}" -f (Get-Date),$_} >> "$using:OutDir\ovr_gpu_metrics.log" }
                }

                Write-Host "Collecting metrics..."

                adb shell "cat /proc/uptime" >> "$OutDir\uptime.log"
                adb shell "cat /proc/net/dev" >> "$OutDir\net_dev.log"
                adb shell "cat /proc/meminfo" >> "$OutDir\meminfo.log"
                adb shell "cat /proc/stat" >> "$OutDir\stat.log"
                adb shell "cat /proc/loadavg" >> "$OutDir\loadavg.log"

                adb shell "dumpsys battery" >> "$OutDir\battery.log"
                adb shell "dumpsys OVRRemoteService" >> "$OutDir\OVRRemoteService.log"
                adb shell "dumpsys CompanionService" >> "$OutDir\CompanionService.log"

                $End = [System.Diagnostics.Stopwatch]::GetTimestamp()
                Do
                {
                    $i = $i + 1
                    $Next = $Start + ($i * $Freq)
                    $Sleep = $Next - $End
                } While ($Sleep -lt 0)
                [System.Threading.Thread]::Sleep($Sleep * (1000.0 / $Freq))
            }

            Write-Host "Done collecting metrics."
        }
        finally
        {
            Write-Host "Stopping VR monitor..."
            Stop-Job $VrJob
            Write-Host "Stopping host monitor..."
            Stop-Job $HostJob
            Write-Host "Stopping OVR GPU monitor..."
            Stop-Job $OVRGPUJob
            
            adb logcat -d | findstr BatteryMgr >> "$OutDir\battery_manager.log"
            Start-Sleep -Seconds 5
            Write-Host "Done collecting BatteryManager metrics."
        }
    }
}
function Get-Duration([System.IO.FileInfo]$TraceFile) { 
    $FirstLine = Get-Content -Head 1 $TraceFile
    $LastLine = Get-Content -Tail 1 $TraceFile
    $FirstTimestamp = [long]($FirstLine.Split(" "))[0]
    $LastTimestamp = [long]($LastLine.Split(" "))[0]
    # Calculate duration, convert nanoseconds to seconds
    return ($LastTimestamp - $FirstTimestamp) / 1000000000
}

try {
    $modeFilePath = "$env:LOCALAPPDATA\librnr\config.txt"

    # Set librnr to configured mode
    Set-Content -Path $modeFilePath -Value "$Mode $TraceFile"

    # Increase logcat buffer size
    adb logcat -G 16M
    
    # Clear logcat buffer
    adb logcat -c

    # Start BatteryManager logging
    ## !! REQUIRES BATTERYMANAGER TO BE INSTALLED AND RUNNING BEFORE OPENING THE OCULUS LINK ON THE VR !!
    adb shell am start-foreground-service -n "com.example.batterymanager_utility/com.example.batterymanager_utility.DataCollectionService" --ei sampleRate $BMSampleRate --es "dataFields" "$BMMetrics" --ez toCSV False
    Write-Host "Starting BatteryManager logging... Sleep for 5 seconds to allow BatteryManager to start."
    Start-Sleep -Seconds 5

    # Create the output directory, if needed
    New-Item $OutDir -ItemType Directory -Force | Out-Null

    # Start tracing
    $TraceJob = Start-Job -InitializationScript $functions -ScriptBlock { Trace-Metrics $using:OutDir $using:PSScriptRoot }

    if ($PSBoundParameters.ContainsKey('SteamAppID') -and $PSBoundParameters.ContainsKey('AppStartupTime')) {
        # Start app via steam. Requires Steam directory to be in the environment variable paths.
        steam steam://rungameid/$SteamAppID
        Write-Host "Starting app... Sleep for $AppStartupTime seconds to allow app to start."
        Start-Sleep -Seconds $AppStartupTime
    } elseif ($PSBoundParameters.ContainsKey('App')) {
        # Start app
        $Process = Start-Process -FilePath $App -PassThru
    }

    if ($Mode -eq "replay") {
        # Obtain length of trace
        $Seconds = (Get-Duration $TraceFile) + 5
        # Wait for the trace to complete
        Write-Output "Will sleep for duration of trace: $Seconds seconds"
        Start-Sleep -Seconds $Seconds
    } else {
        # Recording, sleep until the user stops the script with an interrupt
        Write-Output "Trace recording. Press Ctrl+C to stop the recording."
        while ($True) {
            Start-Sleep 5
            Write-Output "Recording..."
        }
    }
} finally {
    Write-Host "Stopping trace, please wait..."

    if ($PSBoundParameters.ContainsKey('SteamAppExe')) {
        # Stop app after tracing
        taskkill.exe /F /IM "$SteamAppExe"
    } elseif ($PSBoundParameters.ContainsKey('App')) {
        # Stop app after tracing
        Stop-Process -Id $Process.Id -Force -ErrorAction SilentlyContinue
    }

    # Stop tracing
    Stop-Job $TraceJob

    # Stop BatteryManager logging
    Write-Host "Stopping BatteryManager logging..."
    adb shell am stopservice com.example.batterymanager_utility/com.example.batterymanager_utility.DataCollectionService

    # Plot results
    Copy-Item -Path .\README.Rmd $OutDir

    Write-Host "Done $($Mode)ing trace. Open $OutDir\README.Rmd to view results."
}