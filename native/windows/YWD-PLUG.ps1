[CmdletBinding()]
param(
    [ValidateSet('menu','setup','build','install','run','clean','all')]
    [string]$Action = 'menu',
    [switch]$DebugBuild,
    [switch]$InstallMissing,
    [switch]$Yes
)

$ErrorActionPreference = 'Stop'
[Console]::OutputEncoding = [System.Text.UTF8Encoding]::new()

$ProjectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$BuildDir = Join-Path $ProjectRoot 'build'
$DistDir = Join-Path $ProjectRoot 'dist'
$BuildStamp = Join-Path $DistDir 'build-info.txt'
$InstallDir = Join-Path $env:LOCALAPPDATA 'Programs\YWD-Plug'
$BuildType = if ($DebugBuild) { 'Debug' } else { 'Release' }
$script:QtRoot = $null
$script:VsDevCmd = $null

function Write-Banner {
    Clear-Host
    $banner = @(
'$$\     $$\ $$\      $$\ $$$$$$$\          $$$$$$$\  $$\       $$\   $$\  $$$$$$\ '
'\$$\   $$  |$$ | $\  $$ |$$  __$$\         $$  __$$\ $$ |      $$ |  $$ |$$  __$$\'
' \$$\ $$  / $$ |$$$\ $$ |$$ |  $$ |$$$$$$\ $$ |  $$ |$$ |      $$ |  $$ |$$ /  \__|'
'  \$$$$  /  $$ $$ $$\$$ |$$ |  $$ |\______|$$$$$$$  |$$ |      $$ |  $$ |$$ |$$$$\ '
'   \$$  /   $$$$  _$$$$ |$$ |  $$ |        $$  ____/ $$ |      $$ |  $$ |$$ |\_$$ |'
'    $$ |    $$$  / \$$$ |$$ |  $$ |        $$ |      $$ |      $$ |  $$ |$$ |  $$ |'
'    $$ |    $$  /   \$$ |$$$$$$$  |        $$ |      $$$$$$$$\ \$$$$$$  |\$$$$$$  |'
'    \__|    \__/     \__|\_______/         \__|      \________| \______/  \______/ '
    )

    foreach ($line in $banner) { Write-Host $line -ForegroundColor Cyan }
    Write-Host ''
    Write-Host '          NATIVE WINDOWS RADIO PROGRAMMING WORKSTATION' -ForegroundColor Magenta
    Write-Host '          Qt 6  //  C++20  //  DM-32UV  //  KJ6YWD.NET' -ForegroundColor DarkCyan
    Write-Host ('=' * 88) -ForegroundColor DarkGray
}

function Write-Step([string]$Message) { Write-Host ('[>>] ' + $Message) -ForegroundColor Cyan }
function Write-Ok([string]$Message)   { Write-Host ('[OK] ' + $Message) -ForegroundColor Green }
function Write-Warn([string]$Message) { Write-Host ('[!!] ' + $Message) -ForegroundColor Yellow }
function Write-Fail([string]$Message) { Write-Host ('[XX] ' + $Message) -ForegroundColor Red }
function Write-Info([string]$Message) { Write-Host ('     ' + $Message) -ForegroundColor Gray }

function Get-GitIdentity {
    $identity = [ordered]@{ Branch = ''; Sha = ''; Dirty = $false }
    if (-not (Get-Command git.exe -ErrorAction SilentlyContinue)) { return $identity }

    try {
        $branch = (& git.exe -C $ProjectRoot rev-parse --abbrev-ref HEAD 2>$null | Select-Object -First 1)
        $sha = (& git.exe -C $ProjectRoot rev-parse HEAD 2>$null | Select-Object -First 1)
        $dirty = (& git.exe -C $ProjectRoot status --porcelain 2>$null)
        if ($branch) { $identity.Branch = $branch.Trim() }
        if ($sha) { $identity.Sha = $sha.Trim() }
        $identity.Dirty = [bool]$dirty
    } catch {
        # Build/run still works outside a Git checkout; provenance is simply unavailable.
    }
    return $identity
}

function Write-BuildStamp {
    $identity = Get-GitIdentity
    @(
        'YWD-Plug native staged build'
        "branch=$($identity.Branch)"
        "sha=$($identity.Sha)"
        "dirty=$($identity.Dirty)"
        "buildType=$BuildType"
        "builtUtc=$([DateTime]::UtcNow.ToString('o'))"
    ) | Set-Content -Path $BuildStamp -Encoding UTF8
}

function Test-StagedBuildCurrent {
    $exe = Join-Path $DistDir 'YWD-Plug.exe'
    if (-not (Test-Path $exe) -or -not (Test-Path $BuildStamp)) { return $false }

    $identity = Get-GitIdentity
    if (-not $identity.Sha) { return $true }

    $stamp = Get-Content $BuildStamp -ErrorAction SilentlyContinue
    $stagedShaLine = $stamp | Where-Object { $_ -like 'sha=*' } | Select-Object -First 1
    $stagedDirtyLine = $stamp | Where-Object { $_ -like 'dirty=*' } | Select-Object -First 1
    if (-not $stagedShaLine) { return $false }

    $stagedSha = $stagedShaLine.Substring(4).Trim()
    $stagedDirty = if ($stagedDirtyLine) { $stagedDirtyLine.Substring(6).Trim() } else { 'False' }

    if ($stagedSha -ne $identity.Sha) { return $false }
    if ($identity.Dirty -and $stagedDirty -ne 'True') { return $false }
    return $true
}

function Find-VsDevCmd {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (-not (Test-Path $vswhere)) { return $null }

    $installPath = (& $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath 2>$null | Select-Object -First 1)
    if (-not $installPath) { return $null }

    $candidate = Join-Path $installPath 'Common7\Tools\VsDevCmd.bat'
    if (Test-Path $candidate) { return $candidate }
    return $null
}

function Import-VsEnvironment {
    if (Get-Command cl.exe -ErrorAction SilentlyContinue) {
        Write-Ok 'MSVC compiler already available in this shell.'
        return
    }

    if (-not $script:VsDevCmd) { $script:VsDevCmd = Find-VsDevCmd }
    if (-not $script:VsDevCmd) { throw 'Visual Studio 2022 C++ Build Tools were not found.' }

    Write-Step "Loading MSVC environment from $script:VsDevCmd"
    $lines = & cmd.exe /s /c "`"$script:VsDevCmd`" -no_logo -arch=amd64 -host_arch=amd64 && set"
    foreach ($line in $lines) {
        if ($line -match '^([^=]+)=(.*)$') {
            [Environment]::SetEnvironmentVariable($matches[1], $matches[2], 'Process')
        }
    }

    if (-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
        throw 'MSVC environment loaded, but cl.exe is still unavailable.'
    }
    Write-Ok 'MSVC x64 toolchain loaded.'
}

function Find-QtRoot {
    if ($env:QTDIR -and (Test-Path (Join-Path $env:QTDIR 'bin\windeployqt.exe'))) {
        return (Resolve-Path $env:QTDIR).Path
    }

    if ($env:Qt6_DIR -and (Test-Path $env:Qt6_DIR)) {
        $root = Split-Path (Split-Path (Split-Path $env:Qt6_DIR -Parent) -Parent) -Parent
        if (Test-Path (Join-Path $root 'bin\windeployqt.exe')) { return $root }
    }

    $qtBase = Join-Path $env:SystemDrive 'Qt'
    if (Test-Path $qtBase) {
        $versions = Get-ChildItem $qtBase -Directory -ErrorAction SilentlyContinue | Sort-Object Name -Descending
        foreach ($version in $versions) {
            $kits = Get-ChildItem $version.FullName -Directory -ErrorAction SilentlyContinue |
                Where-Object { $_.Name -match '^msvc\d+_64$' } |
                Sort-Object Name -Descending
            foreach ($kit in $kits) {
                if (Test-Path (Join-Path $kit.FullName 'bin\windeployqt.exe')) { return $kit.FullName }
            }
        }
    }

    return $null
}

function Install-CommonTool([string]$Id, [string]$Name) {
    if (-not (Get-Command winget.exe -ErrorAction SilentlyContinue)) {
        Write-Warn "winget is unavailable; cannot install $Name automatically."
        return
    }
    Write-Step "Installing $Name with winget..."
    & winget install --id $Id -e --source winget --accept-package-agreements --accept-source-agreements
    if ($LASTEXITCODE -ne 0) { throw "$Name installation failed." }
}

function Test-Prerequisites([switch]$AllowInstall) {
    Write-Step 'Checking native Windows build prerequisites...'

    foreach ($tool in @(
        @{ Cmd='git.exe'; Name='Git'; Id='Git.Git' },
        @{ Cmd='cmake.exe'; Name='CMake'; Id='Kitware.CMake' },
        @{ Cmd='ninja.exe'; Name='Ninja'; Id='Ninja-build.Ninja' }
    )) {
        if (Get-Command $tool.Cmd -ErrorAction SilentlyContinue) {
            Write-Ok "$($tool.Name) found."
        } elseif ($AllowInstall) {
            Install-CommonTool $tool.Id $tool.Name
        } else {
            throw "$($tool.Name) was not found. Run SETUP.cmd -InstallMissing or install it manually."
        }
    }

    Import-VsEnvironment

    $script:QtRoot = Find-QtRoot
    if (-not $script:QtRoot) {
        throw 'Qt 6 MSVC x64 was not found. Install Qt 6.8+ with an MSVC 2022 64-bit kit, or set QTDIR.'
    }

    $qtConfig = Join-Path $script:QtRoot 'lib\cmake\Qt6\Qt6Config.cmake'
    if (-not (Test-Path $qtConfig)) {
        throw "Qt was found at $script:QtRoot, but Qt6Config.cmake is missing."
    }

    Write-Ok "Qt found: $script:QtRoot"
    Write-Ok 'Native toolchain is ready.'
}

function Invoke-Setup {
    Write-Banner
    Write-Host ' SETUP // TOOLCHAIN PREFLIGHT' -ForegroundColor Magenta
    Write-Host ''
    Test-Prerequisites -AllowInstall:$InstallMissing
    Write-Host ''
    Write-Ok 'Setup complete. Run BUILD.cmd next.'
}

function Invoke-Build {
    Write-Banner
    Write-Host " BUILD // $BuildType" -ForegroundColor Magenta
    Write-Host ''

    Test-Prerequisites -AllowInstall:$false

    # Never leave a previous portable build looking valid while a new compile is in progress.
    # If configure/compile/deploy fails, RUN must not silently launch yesterday's binary.
    if (Test-Path $DistDir) {
        Write-Step 'Invalidating previous staged build...'
        Remove-Item $DistDir -Recurse -Force
    }

    New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null
    Write-Step 'Configuring CMake...'
    & cmake.exe -S $ProjectRoot -B $BuildDir -G Ninja "-DCMAKE_BUILD_TYPE=$BuildType" "-DCMAKE_PREFIX_PATH=$script:QtRoot"
    if ($LASTEXITCODE -ne 0) { throw 'CMake configure failed.' }
    Write-Ok 'CMake configured.'

    Write-Step 'Compiling YWD-Plug...'
    & cmake.exe --build $BuildDir --config $BuildType --parallel
    if ($LASTEXITCODE -ne 0) { throw 'Build failed.' }
    Write-Ok 'Compilation complete.'

    $exe = Join-Path $BuildDir 'YWD-Plug.exe'
    if (-not (Test-Path $exe)) { throw "Build completed but $exe was not found." }

    New-Item -ItemType Directory -Force -Path $DistDir | Out-Null
    Copy-Item $exe (Join-Path $DistDir 'YWD-Plug.exe') -Force

    Write-Step 'Deploying Qt runtime and QML dependencies...'
    $deploy = Join-Path $script:QtRoot 'bin\windeployqt.exe'
    $deployArgs = @('--qmldir', (Join-Path $ProjectRoot 'qml'), '--dir', $DistDir)
    if ($BuildType -eq 'Debug') { $deployArgs += '--debug' } else { $deployArgs += '--release' }
    $deployArgs += (Join-Path $DistDir 'YWD-Plug.exe')
    & $deploy @deployArgs
    if ($LASTEXITCODE -ne 0) {
        if (Test-Path $DistDir) { Remove-Item $DistDir -Recurse -Force }
        throw 'windeployqt failed; staged build removed so RUN cannot launch stale/incomplete output.'
    }

    Write-BuildStamp
    $identity = Get-GitIdentity
    Write-Ok "Portable build staged at: $DistDir"
    if ($identity.Sha) {
        Write-Ok "Staged provenance: $($identity.Branch) @ $($identity.Sha.Substring(0, [Math]::Min(12, $identity.Sha.Length)))"
    }
}

function Invoke-Install {
    Write-Banner
    Write-Host ' INSTALL // CURRENT USER' -ForegroundColor Magenta
    Write-Host ''

    if (-not (Test-StagedBuildCurrent)) {
        Write-Warn 'No current staged build found; building first.'
        Invoke-Build
        Write-Banner
        Write-Host ' INSTALL // CURRENT USER' -ForegroundColor Magenta
        Write-Host ''
    }

    Write-Step "Installing to $InstallDir"
    if (Test-Path $InstallDir) { Remove-Item $InstallDir -Recurse -Force }
    New-Item -ItemType Directory -Force -Path $InstallDir | Out-Null
    Copy-Item (Join-Path $DistDir '*') $InstallDir -Recurse -Force

    $exe = Join-Path $InstallDir 'YWD-Plug.exe'
    $shell = New-Object -ComObject WScript.Shell

    $startMenuDir = Join-Path $env:APPDATA 'Microsoft\Windows\Start Menu\Programs'
    $startShortcut = $shell.CreateShortcut((Join-Path $startMenuDir 'YWD-Plug.lnk'))
    $startShortcut.TargetPath = $exe
    $startShortcut.WorkingDirectory = $InstallDir
    $startShortcut.Description = 'YWD-Plug native radio programming workstation'
    $startShortcut.Save()

    $desktopShortcut = $shell.CreateShortcut((Join-Path ([Environment]::GetFolderPath('Desktop')) 'YWD-Plug.lnk'))
    $desktopShortcut.TargetPath = $exe
    $desktopShortcut.WorkingDirectory = $InstallDir
    $desktopShortcut.Description = 'YWD-Plug native radio programming workstation'
    $desktopShortcut.Save()

    Write-Ok 'Start Menu shortcut created.'
    Write-Ok 'Desktop shortcut created.'
    Write-Ok "Installed: $exe"
}

function Invoke-Run {
    if (-not (Test-StagedBuildCurrent)) {
        Write-Warn 'Staged build is missing or does not match the current source checkout; rebuilding first.'
        Invoke-Build
    }

    $exe = Join-Path $DistDir 'YWD-Plug.exe'
    if (-not (Test-Path $exe)) {
        throw "No runnable staged build exists at $exe."
    }

    if (Test-Path $BuildStamp) {
        Write-Info 'Staged build provenance:'
        Get-Content $BuildStamp | ForEach-Object { Write-Info $_ }
    }

    Write-Step 'Launching YWD-Plug...'
    Start-Process $exe
}

function Invoke-Clean {
    Write-Banner
    Write-Host ' CLEAN // NATIVE BUILD OUTPUT' -ForegroundColor Magenta
    Write-Host ''
    foreach ($path in @($BuildDir, $DistDir)) {
        if (Test-Path $path) {
            Write-Step "Removing $path"
            Remove-Item $path -Recurse -Force
        }
    }
    Write-Ok 'Clean complete.'
}

function Invoke-Menu {
    Write-Banner
    Write-Host ''
    Write-Host '  [1] SETUP      Check native toolchain' -ForegroundColor Cyan
    Write-Host '  [2] BUILD      Compile + stage portable app' -ForegroundColor Cyan
    Write-Host '  [3] INSTALL    Install for current user' -ForegroundColor Cyan
    Write-Host '  [4] RUN        Launch staged build' -ForegroundColor Cyan
    Write-Host '  [5] CLEAN      Remove build outputs' -ForegroundColor Cyan
    Write-Host '  [Q] QUIT' -ForegroundColor DarkGray
    Write-Host ''
    $choice = (Read-Host 'YWD-PLUG>').Trim().ToUpperInvariant()
    switch ($choice) {
        '1' { Invoke-Setup }
        '2' { Invoke-Build }
        '3' { Invoke-Install }
        '4' { Invoke-Run }
        '5' { Invoke-Clean }
        default { return }
    }
}

try {
    switch ($Action) {
        'setup'   { Invoke-Setup }
        'build'   { Invoke-Build }
        'install' { Invoke-Install }
        'run'     { Write-Banner; Invoke-Run }
        'clean'   { Invoke-Clean }
        'all'     { Invoke-Setup; Invoke-Build; Invoke-Install }
        default   { Invoke-Menu }
    }
} catch {
    Write-Host ''
    Write-Fail $_.Exception.Message
    Write-Host ''
    Write-Host 'Build stopped. Fix the item above and run the command again.' -ForegroundColor Yellow
    exit 1
}
