We automate the creation of a virtualbox VM on a Windows 11 host.
The VM runs Ubuntu 22.04 and is used to emulate networks.
We loop Windows' network through this VM to make the network emulation work for VR applications.

Requirements:
1. Windows PowerShell with permissions to execute PowerShell scripts
2. Have sshd enabled and a ~/.ssh folder
3. Have Virtualbox installed, at least version 7
4. Have Python3 installed. You can install this via the Microsoft Store
5. Have the Microsoft Defender Network Firewall disabled. Otherwise you won't be able to detect devices on the network from your VM

To run the script, from Powershell:
.\create-vm.ps1