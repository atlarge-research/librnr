# See the README for an explanation

function Enable-NetBalancer {
    # Enable the use of NetBalancer via the command line
    $exe_path = "C:\Program Files\NetBalancer"
    $env:PATH = $env:PATH + ";" + $exe_path
}

function Reset-NetBalancer {
    # Reset NetBalancer for new use
    nbcmd settings reset
}

function Set-ThroughputLimit {
    # Reset NetBalancer for new use
    param (
        [Parameter(Mandatory = $true, Position = 0)]
        [int] $up,
        [Parameter(Mandatory = $true, Position = 1)]
        [int] $down
    )
    # From mbit to bytes
    $up = ($up * [Math]::Pow(10, 6)) / 8
    $down = ($down * [Math]::Pow(10, 6)) / 8

    # The file to replace in
    $path = (Get-Location).Path
    $file = $path + "\net-rule.json"

    # The strings to replace
    $string_up = '"DownloadLimit": '
    $string_down = '"UploadLimit": '

    # Grep the line to replace
    $line_up = Get-Content $file | Select-String $string_up | Select-Object -ExpandProperty Line
    $line_down = Get-Content $file | Select-String $string_down | Select-Object -ExpandProperty Line

    # Create the same lines but with different throughput values
    $replace_up = "        " + $string_up + $up + ","
    $replace_down = "        " + $string_down + $down + ","

    # Write the updated values to the file
    $output = Get-Content $file | ForEach-Object { $_ -replace $line_up, $replace_up }
    $output | Set-Content $file

    $output = Get-Content $file | ForEach-Object { $_ -replace $line_down, $replace_down } 
    $output | Set-Content $file

    # Load the file with new throughput settings into the emulator
    nbcmd settings files rules load $file
}

Enable-NetBalancer
Reset-NetBalancer
Set-ThroughputLimit $args[0] $args[1]