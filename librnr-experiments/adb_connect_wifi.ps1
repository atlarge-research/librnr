# Start the BatteryManager app
adb shell am start -n "com.example.batterymanager_utility/com.example.batterymanager_utility.MainActivity" -a android.intent.action.MAIN -c android.intent.category.LAUNCHER

# Get IP address of the device
$ipAddress = adb shell ip -f inet addr show wlan0 | Select-String -Pattern 'inet\s+\d+\.\d+\.\d+\.\d+' | ForEach-Object { $_ -replace '.*inet\s+(\d+\.\d+\.\d+\.\d+).*', '$1' }

# Remove whitespace if IP address is found
$ipAddress = $ipAddress.Trim() + ":5555"

if (-not [string]::IsNullOrWhiteSpace($ipAddress)) {
    # Connect to the device via Wi-Fi
    adb tcpip 5555
    Start-Sleep -Seconds 2 # Wait a bit for the device to start listening for Wi-Fi connections
    adb connect $ipAddress

    # Check connection status
    $connectedDevices = adb devices
    Write-Output $connectedDevices
} else {
    Write-Output "Failed to retrieve the device IP address. Please check the device connection and settings."
}
