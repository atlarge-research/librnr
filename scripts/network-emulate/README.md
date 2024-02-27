# Network Emulation
What this repo does:
1. Static network emulation on Windows. Specifically, set a maximum bandwidth for this computer.
2. Validate the network settings via an iperf3 benchmark.

If you are only interested in this, this README.md file and net-delay-static.ps1 are the only important files.
The other files in this repository have different functions, see other directories and their README's.

## Requirements
1. Windows PowerShell, at least version 5 (backwards compatible, newer versions are fine).
2. Administrator rights.
3. NetLimiter: https://www.netlimiter.com/. The free version is fine.

## Preparation
Once NetLimiter is installed, open its GUI. In the activity tab, click on the entry at the top with your computer's name. A new sub-window should open at the right called "Info View". Here, set the In and Out Limit to any value. Next, go to the "Rule List" tab, right next to the "Activity" tab. There should be 4 rules in total: Two that ignore all traffic, and two with the in and out bandwidth limit you just set. Make sure these two rules have "Active" as State.

## Usage
Open PowerShell as administrator, and navigate to this directory

```
# The script requires 1 argument, which is upload/download throughput in mbit/s
.\net-delay-static.ps1 100
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

# You can also switch the sender and receiver side, use the -R option on the client.
```

## How to Stop the Tool 
You can either execute `net stop nlsvc` from the PowerShell Administrator terminal, or go to the NetLimiter GUI and make sure the Limit Type rules in the Rule List tab have the Disabled state.