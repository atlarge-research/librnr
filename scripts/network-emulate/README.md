# Network Emulation
1. We automate the creation of a VirtualBox VM (Ubuntu 22.04) on a Windows 11 host
2. We route the network between the host and the VR headset through the VM
3. We execute network emulation software (MahiMahi) on the VM to emulate networks that we don't have in hardware

## Current Status
* A script can automatically start a VM with correct network configurations and internet access. The VM can be accessed via SSH.

## Requirements
1. Windows PowerShell with permissions to execute PowerShell scripts
2. Have sshd enabled and a ~/.ssh folder
3. Have Virtualbox installed, at least version 7
4. Have Python3 installed. You can install this via the Microsoft Store
5. Have the Microsoft Defender Network Firewall disabled. Otherwise you won't be able to detect devices on the network from your VM

## Usage
From Powershell: `.\create-vm.ps1`
