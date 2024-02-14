# Network bypassing through VM
THIS README IS OUTDATED


What this repo does:
1. Create a VirtualBox VM with Ubuntu 22.04 on a Windows 11 host
2. Route the network between the host and the VR headset through the VM
3. Execute network emulation software (MahiMahi) on the VM to emulate networks between the PC and VR headset

## Usage
1. Download `metabench.ova` from Google Drive: https://drive.google.com/drive/folders/1YxRBExNQJShqcZJcoIcfZ6KbH5bMh0HB?usp=sharing. This is a 2 GB file. You can save it in any directory.
2. Open VirtualBox and create a VM using the downloaded file. Use File -> Import Appliance. Update the network settings of the VM: Settings -> Network. Adapter 1 should be attached to a host-only adapter, specifically Virtualbox' host-only network, which is created when you install Virtualbox. Adapter 2 should be attached to a bridge, specifically the network adapter that your computer currently uses to get internet.
3. Start the VM in VirtualBox. Wait for the VM to boot and login with username and password "metabench".
4. Verify the VM's network is set up properly. Use `ping google.com` to check if internet works. Then, use `ip a` and verify there are 3 networks, *lo*, *enp0s3* (with IP 192.168.56.10, which is used to communicate between the host PC and the VM), and *enp0s8* (with an IP given by your local DHCP server, which you use to get internet on the VM).
5. Enable ipv4 fowarding in the VM:
    ```
    sudo sed -i 's/#net.ipv4.ip_forward=1/net.ipv4.ip_forward=1/' /etc/sysctl.conf
    sudo sysctl -p
    ```
6. Enable NAT on the bridged interface using iptables
    ```
    sudo iptables -t nat -I POSTROUTING --out-interface enp0s8 -j MASQUERADE
    ```
7. Open Windows PowerShell as administrator on the host. Use `Get-NetRoute` to list the network routes of the host. Search for the rule with as DestinationPrefix 0.0.0.0/0 and as NextHop something similar 192.168.1.1. This is the route that connects your PC to the internet. Delete it with `Remove-NetRoute -DestinationPrefix “0.0.0.0/0” -InterfaceIndex X`, with X being the ifIndex of the rule you just identified.
8. Now, we create a new route that moves the host's internet through the VM. In the same PowerShell terminal, run `New-NetRoute -DestinationPrefix "0.0.0.0/0" -InterfaceIndex X -NextHop Y -ROuteMetric 1`. Here, X is the ifIndex of the route that your host-only adapter uses, which has a DestinationPrefix of 192.168.56.1/32 by default, unless you have modified VirtualBox's host-only adapter. Y is the IP of the VM, which is 192.168.56.10 by default, see step 4. Verify the routing table: Run `Get-NetRoute`, there should be only one entry with DestinationPrefix 0.0.0.0/0, and it should have a NextHop of 192.168.56.10. Test the setup by opening any webpage on any browser on your host. You should have internet, and if so, it is being routes through the VM.

## Requirements
1. Have Virtualbox installed, at least version 7.
2. Windows PowerShell. We have tested with versions 5 and 7.