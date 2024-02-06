# Network Emulation
What this repo does:
1. Create a VirtualBox VM with Ubuntu 22.04 on a Windows 11 host
2. Route the network between the host and the VR headset through the VM
3. Execute network emulation software (MahiMahi) on the VM to emulate networks between the PC and VR headset

## Usage
1. Download `metabench.ova` from Google Drive: https://drive.google.com/drive/folders/1YxRBExNQJShqcZJcoIcfZ6KbH5bMh0HB?usp=sharing. This is a 2 GB file. You can save it in any directory.
2. Open VirtualBox and create a VM using the downloaded file. Use File -> Import Appliance. Update the network settings of the VM: Settings -> Network. Make sure that the Bridged Adapter (Adapter 2) is connected to the network adapter that your computer currently uses to get internet. Click on the Name dropdown menu to see the options.
3. TBC




<!-- ## Requirements
1. Windows PowerShell with permissions to execute PowerShell scripts. We use PowerShell version 5, which is compatible with future versions. When executing the script mentioned in *Usage*, and PowerShell permission are not set correctly, Windows will explain how to update permissions. We recommend `Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser` for the most restricte policy that still allows execution of PowerShell scripts.
2. Have sshd enabled and a ~/.ssh folder.
3. Have Virtualbox installed, at least version 7.
4. Have Python3 installed. You can install this via the Microsoft Store. For example, type `python3` in PowerShell, and it will automatically open the Store for you.
5. If you are on a protected network (e.g., university network), disable the Microsoft Defender Network Firewall. Otherwise, you won't be able to detect devices on the network from the VM, which includes the netboot server we use to boot the VM. If your network is a private network, you only need to disable the firewall of the private network.
6. The name of your internet adapter, see the `$global:bridged` variable in `create-vm.ps1`. To list your network adapaters, search for "view network connections" in the Windows toolbar. Choose the network adapter that you get internet from. Not all adapters will work, we need an adapter that supports bridging (e.g., USB-to-Ethernet adapters often don't support this).

## Usage
From Powershell: `.\create-vm.ps1`


## Known Errors
You may encounter errors during the execution of the script. We list some known errors below and how to resolve them.

1. `VBoxManage.exe: error: Not in a hypervisor partition (HVP=0) (VERR_NEM_NOT_AVAILABLE). VBoxManage.exe: error: AMD-V is disabled in the BIOS (or by the host OS) (VERR_SVM_DISABLED)`. You need to enable AMD-V (or VT-x for Intel CPUs) in the Bios. These are the hardware acceleration features the CPU offers to execute VMs at acceptable performance. Without these options, the entire VM needs to be emulated, resulting in very slow performance. -->