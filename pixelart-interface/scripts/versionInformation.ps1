$basePath = Split-Path -Parent (Split-Path -Parent $PSCommandPath)
$buildfolder = "$basePath\build"
$versionFile = "$buildfolder\version.json"
$templateFile = "$basePath\version.json"
$envFile = "$basePath\.env"
$env = @{}

if((Test-Path -Path $templateFile) -and (Test-Path -Path $envFile)) {
    Copy-Item $templateFile $versionFile
    Get-Content $envFile | foreach {
        $name, $value = $_.split('=')
        $env[$name] = $value
    }
    cd $buildfolder
    $json = Get-Content $versionFile | ConvertFrom-Json
    $json.version = $env.REACT_APP_VERSION
    $json.environment = $env.REACT_APP_ENVIRONMENT
    $json.files = get-childitem -file -path .\ -Recurse | Resolve-Path -Relative | % { if(!$_.StartsWith(".\")) {$_ = ".\$($_)"}; "$(($_ -replace "\\", "/") -replace "\.\/", '')" } | ? { $_ -notlike "*.htaccess*" -and $_ -notlike "*robots.txt*" }
    $json | ConvertTo-Json -depth 32 | Out-File $versionFile
} else {
    Write-Error "Version file in build folder does not exist"
}

cd $basePath