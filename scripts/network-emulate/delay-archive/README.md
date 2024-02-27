# Network Emulation
What this repo does:
1. Static network emulation on Windows. Specifically, set a maximum bandwidth for this computer.
2. Validate the network settings via an iperf3 benchmark.

If you are only interested in this, this README.md file and net-delay-static.ps1 are the only important files.
The other files in this repository have different functions, see other directories and their README's.

## Requirements
1. Windows PowerShell, at least version 5 (backwards compatible, newer versions are fine).
2. NetBalancer: https://netbalancer.com/. The free version is fine.

## Usage
```
# In PowerShell, this directory
# The script requires 2 argument, which is upload/download throughput in mbit/s
.\net-delay-static.ps1 100 100

# You can verify the settings as follows
nbcmd settings
    ...
    [Default priority]
    Download priority: Limited
    Download delay: 0
    Download drop rate: 0
    Download limit: 12500000
    Upload priority: Limited
    Upload delay: 0
    Upload drop rate: 0
    Upload limit: 12500000
    Limit per connection: False
    ...

# To reset all limits
nbcmd settings reset
```

## Benchmarking
You require 2 computers: This computer (host) and a remote. 
Both computers can be VMs as well, as long as they can communicate directly.

```
# On both computers
# Install iperf3: https://iperf.fr/iperf-download.php

# On the remote PC, start the server
iperf3 -s -p 7979

# On the host PC
# For TCP benchmarking
iperf3 -c <IP> -p 7979

# For UDP benchmarking
iperf3 -c <IP> -p 7979 -u -b 100M

# You can get the IP of a PC as follows
# On Windows, using PowerShell
(Get-NetIPConfiguration |
    Where-Object {
        $null -ne $_.IPv4DefaultGateway -and
        $_.NetAdapter.Status -ne "Disconnected"
    }
).IPv4Address.IPAddress

# On Ubuntu
ip a


# You can also switch the sender and receiver side
# Just swap the two iperf3 commands
```
