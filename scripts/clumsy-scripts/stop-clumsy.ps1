try{
    $clumsy = Get-Process clumsy -ErrorAction Stop
    if ($clumsy) {
        $clumsy | Stop-Process -Force
    }
    Remove-Variable clumsy
}
catch {
    echo "Could not find or close clumsy process. Check if it is running and close it manually, or try again with Run As Administrator."
}
