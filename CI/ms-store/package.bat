:: .\package.bat "RelWithDebInfo" "build"

@echo off

IF NOT "%1" == "" (SET BUILD_TYPE=%1) ELSE (SET BUILD_TYPE=RelWithDebInfo)

IF NOT "%2" == "" (SET BUILD_DIR=%2) ELSE (SET BUILD_DIR=build)

SET KIT_PATH=C:\Program Files (x86)\windows kits\10\bin\10.0.19041.0\x64

:: %MPRI_PATH%\makepri.exe new /pr "../../%3/rundir/%2" /cf priconfig.xml
IF NOT EXIST ".\newbuild" mkdir newbuild
IF NOT EXIST ".\newbuild\bin" mkdir newbuild\bin
IF NOT EXIST ".\newbuild\data" mkdir newbuild\data
IF NOT EXIST ".\newbuild\obs-plugins" mkdir newbuild\obs-plugins
echo Copying folders:
robocopy "..\..\%BUILD_DIR%\rundir\%BUILD_TYPE%\bin" "newbuild\bin" /e /NJH /NJS
robocopy "..\..\%BUILD_DIR%\rundir\%BUILD_TYPE%\data" "newbuild\data" /e /NJH /NJS
robocopy "..\..\%BUILD_DIR%\rundir\%BUILD_TYPE%\obs-plugins" "newbuild\obs-plugins" /e /NJH /NJS
echo Building resources.pri using makepri.exe
"%KIT_PATH%\makepri.exe" new /pr newbuild /cf priconfig.xml

echo == You can run `Add-AppPackage -Register .\AppxManifest.xml` in PowerShell to test the package

copy AppxManifest.xml newbuild\AppxManifest.xml
copy obs.png newbuild\obs.png
echo Making msix file
"%KIT_PATH%\makeappx.exe" pack /d newbuild /p OBS.msix
:: TODO signtool.exe
:: "%KIT_PATH%\signtool.exe sign /fd SHA256 /f cert.pfx /p mypass OBS.msix"