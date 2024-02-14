# See the README for an explanation

function Stop-NetLimiter {
    # Stop NetLimiter for new use
    net stop nlsvc
}

function Start-NetLimiter {
    # Start NetLimiter for new use
    net start nlsvc
}

function Set-ThroughputLimit {
    # Set throughput limit
    param (
        [Parameter(Mandatory = $true, Position = 0)]
        [int] $throughput
    )
    # From mbit to bytes
    $throughput = ($throughput * [Math]::Pow(10, 6)) / 8

    # File with NetLimiter settings
    $file = "C:\ProgramData\Locktime\NetLimiter\5\nl_settings.xml"

    # Replace
    $xml = [xml](Get-Content -Path $file)
    $node = $xml.NLSvcSettings.Rules.Rule | Where-Object { $_.type -eq 'limitRule' }
    $node | ForEach-Object { $_.limitSize = "$throughput" }
    $xml.Save($file)
}

Stop-NetLimiter
Set-ThroughputLimit $args[0]
Start-NetLimiter
