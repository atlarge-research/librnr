param (
    [Parameter(Mandatory=$true)][string]$apk,
    [Parameter(Mandatory=$true)][string]$libdir
)

function New-TemporaryDirectory {
    $parent = [System.IO.Path]::GetTempPath()
    [string] $name = [System.Guid]::NewGuid()
    New-Item -ItemType Directory -Path (Join-Path $parent $name)
}

if (-Not (Test-Path env:JAVA_HOME)) {
    echo "env:JAVA_HOME must be set"
    exit 1
}

$tmpdir = New-TemporaryDirectory
mkdir $tmpdir/unzipped
$apkname = (Get-ChildItem $apk).BaseName
$apkzipname = $apkname'.zip'
Copy-Item $apk $tmpdir'/'$apkzipname
Set-Location $tmpdir/unzipped
7z x '../'$apkzipname
mkdir .\assets\openxr\1\api_layers\implicit.d
cp $libdir/manifest.json .\assets\openxr\1\api_layers\implicit.d
cp $libdir/librnr.so .\assets\openxr\1\api_layers\implicit.d
Remove-Item -Path META-INF\*.*
$newzipname = $apkname'.new.zip'
7z a $newzipname *
mv $newzipname ..
cd ..
$newapkname = $apkname'.new.apk'
mv $newzipname $newapkname
$buildtools = (dir C:\Users\atl_gaming\AppData\Local\Android\Sdk\build-tools\).Name
$alignedapkname = $apkname'.new.aligned.apk'
Invoke-Expression $env:LocalAppData\Android\Sdk\build-tools\$buildtools\zipalign -v 4 $newapkname $alignedapkname
