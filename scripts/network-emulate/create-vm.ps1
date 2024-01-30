# -----------------------------------
# This script is responsible for creating a VirtualBox VM on Windows 11.
# The VM will later be used for run MahiMahi in to emulate (cellular) networks between a VR headset 
# and windows host. 
# -----------------------------------

function Enable-Virtualbox {
    # Enable the use of VirtualBox via the command line
    $virtualbox_exe_path = "C:\Program Files\Oracle\VirtualBox"
    $env:PATH = $env:PATH + ";" + $virtualbox_exe_path
}

function Stop-BackgroundProcs {
    # Stop all running background jobs
    Write-Host("Stop all running background jobs in this terminal")
    Get-Job | Stop-Job
    Get-Job | Remove-Job
}

function Remove-ActiveVms {
    # Delete active VMs
    $output = vboxmanage list runningvms
    foreach ($out in $output) {
        if ($out.Contains($global:vm) -Or $out.Contains("inaccessible")) {
            $uuid = ($out -split " {", 2)[1]
            $uuid = ($uuid -split "}", 2)[0]

            # Running this command with 2>&1 makes it always crash. So we just pray.
            Write-Host("Stop running VM")
            $null = vboxmanage controlvm $uuid poweroff 2>&1
            Start-Sleep 3
        }
    }
}

function Remove-Vms {
    # Delete all VMs that are inaccessible or are titled "metabench"
    $output = vboxmanage list vms
    foreach ($out in $output) {
        if ($out.Contains($global:vm) -Or $out.Contains("inaccessible")) {
            $uuid = ($out -split " {", 2)[1]
            $uuid = ($uuid -split "}", 2)[0]

            # Running this command with 2>&1 makes it always crash. So we just pray.
            Write-Host("Delete existing VM")
            $null = vboxmanage unregistervm $uuid --delete 2>&1
        }
    }
}

function Remove-Disks {
    # Delete disks
    $output = vboxmanage list hdds
    $uuid = ""
    $last_deleted_uuid = ""
    $path = "\" + $global:vm + ".vdi"
    foreach ($out in $output) {
        if ($out.Contains("UUID:") -And -not($out.Contains("Parent "))) {
            $uuid = ($out -split "UUID:", 2)[1].trim()
        }
        elseif ($out.Contains("State:")) {
            Write-Host("Delete inaccessible VM image")
            $null = vboxmanage closemedium disk $uuid --delete 2>&1
            $last_deleted_uuid = $uuid
        }
        elseif ($out.Contains("Location:") -And $out.Contains($path) `
                -And $uuid -ne $last_deleted_uuid) {
            Write-Host("Delete metabench VM image")
            $null = vboxmanage closemedium disk $uuid --delete 2>&1
            $last_deleted_uuid = ""
        }
    }
}

function Remove-DirVm {
    # Delete directory for virtualbox vm
    $path = $global:host_base_path + "\" + $global:vm
    if (Test-Path -LiteralPath $path) {
        Write-Host("Delete existing directory: " + $path)
        Remove-Item -LiteralPath $path -Recurse
    }
}

function Remove-Vdi {
    # Delete VDI if it does not yet exist
    $path = $global:host_base_path + "\" + $global:vm + ".vdi"
    if (Test-Path -LiteralPath $path) {
        Write-Host("Delete existing file: " + $path)
        Remove-Item -LiteralPath $path
    }
}

function Remove-DirConfig {
    # Delete directory for vm boot config
    $path = $global:host_base_path + "\config"
    if (Test-Path -LiteralPath $path) {
        Write-Host("Delete existing directory: " + $path)
        Remove-Item -LiteralPath $path -Recurse
    }
}

function Remove-All {
    # Combine all remove commands in one function
    Remove-ActiveVms
    Remove-Vms
    Remove-Disks
    Remove-DirVm
    Remove-Vdi
    Remove-DirConfig
}

function New-Vm {
    # Create the VM, only on a metadata level
    Write-Host "Create the VM"
    $output = vboxmanage createvm --name $global:vm --ostype Ubuntu22_LTS_64 --register `
        --basefolder $(Get-Location).Path 2>&1
    if ($output[0].GetType().Name -contains "ErrorRecord") {
        Write-Host("`tERROR: Could not create VM")
        Write-Host($output)
        Exit
    }
}

function Get-Iso {
    # Fetch the ISO for Ubuntu 2204
    param (
        [Parameter(Mandatory = $true, Position = 0)]
        [string] $iso,
        [Parameter(Mandatory = $true, Position = 1)]
        [string] $isopath
    )
    if (-not (Test-Path -LiteralPath $isopath)) {
        Write-Host("Download VM ISO")
        $ProgressPreference = 'SilentlyContinue'
        $url = "https://releases.ubuntu.com/jammy/ubuntu-22.04.3-live-server-amd64.iso"
        Invoke-WebRequest $url -O $iso
    }
    Write-Host("VM ISO is present")
}

function Set-VmCharacteristics {
    # Set various VM characteristics
    Write-Host("Set VM characteristics")
    $hostonly = "VirtualBox Host-Only Ethernet Adapter"
    $bridged = "Remote NDIS based Internet Sharing Device"
    vboxmanage modifyvm $global:vm --cpus 2
    vboxmanage modifyvm $global:vm --memory 2048
    vboxmanage modifyvm $global:vm --vram 128
    vboxmanage modifyvm $global:vm --acpi on
    vboxmanage modifyvm $global:vm --ioapic on
    vboxmanage modifyvm $global:vm --rtc-use-utc on
    vboxmanage modifyvm $global:vm --nic1 hostonly --hostonlyadapter1 $hostonly
    vboxmanage modifyvm $global:vm --nic2 bridged --bridgeadapter2 $bridged
    vboxmanage modifyvm $global:vm --graphicscontroller vmsvga
}

function Set-VmStorage {
    # Set VM storage
    Write-Host("Set VM storage")
    $filename = $global:host_base_path + "\" + $global:vm + ".vdi"
    vboxmanage storagectl $global:vm --name "SATA" --add sata --bootable on --hostiocache off
    $null = vboxmanage createmedium --filename $filename --size 30000 --variant Standard 2>&1
    vboxmanage storageattach $global:vm --storagectl "SATA" --port 0 --device 0 `
        --type hdd --medium $filename
}

function New-SshKey {
    # Create a new SSH key to use with the VM
    param (
        [Parameter(Mandatory = $true)]
        [string] $sshpath
    )
    if (-not (Test-Path -LiteralPath $sshpath)) {
        Write-Host("Create SSH key")
        $output = ssh-keygen -t ed25519 -f "$sshpath" -N '"' -q 2>&1
        if ($output[0].GetType().Name -contains "ErrorRecord") {
            Write-Host("`tERROR: Could not create VM")
            Write-Host($output)
            Exit
        }

        # Set correct permissions for private key
        $null = icacls $sshpath /inheritance:r
        start-process "icacls.exe" -ArgumentList '$sshpath /grant:r "$env:USERNAME":"(R)"'

        $sshpath = $sshpath + ".pub"
        $null = icacls $sshpath /inheritance:r
        start-process "icacls.exe" -ArgumentList '$sshpath /grant:r "$env:USERNAME":"(R)"'
    }
}

function New-VmConfig {
    # Create the cloud-config to boot the VM with
    param (
        [Parameter(Mandatory = $true, Position = 0)]
        [string] $sshpath,
        [Parameter(Mandatory = $true, Position = 1)]
        [string] $configpath,
        [Parameter(Mandatory = $true, Position = 2)]
        [string] $username,
        [Parameter(Mandatory = $true, Position = 3)]
        [string] $vmip,
        [Parameter(Mandatory = $true, Position = 4)]
        [string] $vmgateway
    )
    Write-Host("Prepare VM config")
    $sshpath = $sshpath + ".pub"
    $sshkey = Get-Content $sshpath

    # Get hash for password $username
    $stringAsStream = [System.IO.MemoryStream]::new()
    $writer = [System.IO.StreamWriter]::new($stringAsStream)
    $writer.write("$username")
    $writer.Flush()
    $stringAsStream.Position = 0
    $hash = (Get-FileHash -InputStream $stringAsStream -Algorithm SHA512).Hash

    $content = @"
#cloud-config
autoinstall:
  version: 1
  identity:
    hostname: $global:vm
    password: "$hash"
    username: $username
  ssh:
    install-server: true
    authorized-keys: 
      - $sshkey
    allow-pw: false
  user-data:
    disable-root: false
  network:
    version: 2
    ethernets:
      enp0s3:
        dhcp4: false
        addresses:
        - $vmip/24
        nameservers:
          addresses: []
          search: []
        routes:
        - to: default
          via: $vmgateway
          metric: 200
      enp0s8:
        dhcp4: true
        dhcp4-overrides:
          route-metric: 100
"@

    if (-not (Test-Path -LiteralPath $configpath)) {
        $null = New-Item -ItemType Directory -Path $configpath
    }
    $configpath_extend = $configpath + "\user-data"
    Add-Content -Path $configpath_extend -Value $content
}

function New-PythonServer {
    # Start a Python server
    param (
        [Parameter(Mandatory = $true)]
        [string] $configpath
    )

    Write-Host("Start Python server in background for net-booting")
    $port = 3003
    $hostip = (
        Get-NetIPConfiguration |
        Where-Object {
            $null -ne $_.IPv4DefaultGateway -and
            $_.NetAdapter.Status -ne "Disconnected"
        }
    ).IPv4Address.IPAddress
    $ipport = $hostip + ":" + $port

    $output = Start-Job -ScriptBlock { 
        Set-Location $using:configpath; python3 -m http.server $using:port 
    }
    Write-Host("You can verify the server works as intended with: curl http://$ipport/user-data")
    $id = $output.Id
    Write-Host("When the script crashes, you can manually stop the server with: Stop-Job -Id $id")

    return $ipport
}

function Start-UnattendedInstall {
    # Begin VM boot
    param (
        [Parameter(Mandatory = $true, Position = 0)]
        [string] $ipport,
        [Parameter(Mandatory = $true, Position = 1)]
        [string] $isopath
    )
    Write-Host("Begin the unattended install of the Ubuntu 22.04 VM")
    $param = "autoinstall cloud-config-url=http://$ipport/user-data ds=nocloud-net;s=http://$ipport/"
    vboxmanage unattended install $global:vm --iso $isopath `
        --extra-install-kernel-parameters=$param --start-vm=headless
}

function Wait-Init {
    # Wait for the VM to have fully booted
    Write-Host("Wait for the VM to have initialized")
    Start-Sleep 10
    while ($true) {
        $output = vboxmanage guestproperty enumerate $global:vm
        $out = ($output[6] -split "= '", 2)[1]
        $reset = ($out -split "'   ", 2)[0]
        if ($reset -eq "0") {
            Start-Sleep 5
        }
        else {
            break
        }
    }
    Write-Host("VM has been initialized")
}

function Show-SshAccess {
    # Make sure we can SSH to the VM from the host
    param (
        [Parameter(Mandatory = $true, Position = 0)]
        [string] $sshpath,
        [Parameter(Mandatory = $true, Position = 1)]
        [string] $vmip,
        [Parameter(Mandatory = $true, Position = 2)]
        [string] $username
    )
    Write-Host("Enable SSH access")
    Write-Host("Login with: ssh $username@$vmip -o StrictHostKeyChecking=no -i $sshpath")
}


########################################################
# TODO: verify these have been fixed
$global:vm = "metabench"
$global:host_base_path = $(Get-Location).Path

Enable-Virtualbox
Stop-BackgroundProcs
Remove-All
New-Vm

$iso = "ubuntu2204.iso"
$isopath = $global:host_base_path + "\" + $iso
Get-Iso $iso $isopath

Set-VmCharacteristics
Set-VmStorage

$sshpath = $env:USERPROFILE + "\.ssh\id_ed25519_metabench"
$configpath = $global:host_base_path + "\config"
$username = "admin"
$vmip = "192.168.56.10"
$vmgateway = "192.168.56.1"

New-SshKey $sshpath
New-VmConfig $sshpath $configpath $username $vmip $vmgateway
[string] $ipport = New-PythonServer $configpath
Start-UnattendedInstall $ipport $isopath
Wait-Init
Show-SshAccess $sshpath $vmip $username

#-----------------------------------
# vboxmanage guestcontrol metaverse copyto --target-directory=/home/metabench .\setup.sh 
# vboxmanage guestcontrol $global:vm run --exe /home/metabench/setup.sh --username root 