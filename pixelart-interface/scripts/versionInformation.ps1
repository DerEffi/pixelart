$basePath = Split-Path -Parent (Split-Path -Parent $PSCommandPath)
$buildfolder = "$basePath\build"
$versionFile = "$buildfolder\interface.version.json"
$envFile = "$basePath\.env"
$env = @{}

if((Test-Path -Path $versionFile) -and (Test-Path -Path $envFile)) {
    Get-Content $envFile | foreach {
        $name, $value = $_.split('=')
        $env[$name] = $value
    }
    cd $buildfolder
    $json = Get-Content $versionFile | ConvertFrom-Json
    $json.version = $env.REACT_APP_VERSION
    $json.type = $env.REACT_APP_ENVIRONMENT
    $json.files = get-childitem -file -path .\ -Recurse | Resolve-Path -Relative | % { ($_ -replace "\\", "/") -replace "\.\/", "/" }
    $json | ConvertTo-Json -depth 32 | Out-File $versionFile
} else {
    Write-Error "Version file in build folder does not exist"
}