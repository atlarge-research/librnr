# To reset all changes
nbcmd settings reset

$up = "Verizon-LTE-driving.up"
$down = "Verizon-LTE-driving.down"

$string_up = '"DownloadLimit": '
$string_down = '"UploadLimit": '

# Add later perhaps: Latency via these two parameters
# DownloadDelay
# UploadDelay

$file = "C:\Users\redpl\phd\vr\librnr\scripts\network-emulate\net-rule.json"

$lines_up = Get-Content $up
$lines_down = Get-Content $down

# TODO
# We swap download and upload because the MahiMahi traces were measures from a 4G base station
# and we are executing from a PC, so it's a different point so swapped in and out
for ($i = 0; $i -lt $lines_up.Length; $i++) {
    # Grab the input and output lines from the file
    $line_up = Get-Content $file | Select-String $string_up | Select-Object -ExpandProperty Line
    $line_down = Get-Content $file | Select-String $string_down | Select-Object -ExpandProperty Line

    # Create the same lines but with different throughput values
    $replace_up = "        " + $string_up + $lines_up[$i] + ","
    $replace_down = "        " + $string_down + $lines_down[$i] + ","

    # Write the updated values to the file
    $output = Get-Content $file | ForEach-Object { $_ -replace $line_up, $replace_up }
    $output | Set-Content $file

    $output = Get-Content $file | ForEach-Object { $_ -replace $line_down, $replace_down } 
    $output | Set-Content $file

    # Load the file with new throughput settings into the emulator
    nbcmd settings files rules load $file

    # Sleep 1 second and repeat
    Start-Sleep -Seconds 1
}

# Limit latency to 100 ms
# nbcmd settings performance latency 100

# To view the current settings
# nbcmd settings adapaters