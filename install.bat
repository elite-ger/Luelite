@echo off
title Luelite Installer
echo ========================================
echo   Luelite 0.1 Installer
echo ========================================
echo.

set "USERPROFILE=%USERPROFILE%"
set "INSTALLDIR=%USERPROFILE%\retrelite"

echo User: %USERNAME%
echo Install to: %INSTALLDIR%
echo.

if not exist "%INSTALLDIR%" (
    echo Creating %INSTALLDIR%...
    mkdir "%INSTALLDIR%"
)

echo Copying luelite.exe...
copy /Y "%~dp0luelite.exe" "%INSTALLDIR%\luelite.exe"

if errorlevel 1 (
    echo ERROR: Failed to copy luelite.exe
    pause
    exit /b 1
)

echo Adding to PATH...
set "PATH_CHECK=%PATH%"
echo %PATH_CHECK% | find /I "%INSTALLDIR%" >nul
if errorlevel 1 (
    setx PATH "%PATH%;%INSTALLDIR%" >nul
    echo PATH updated.
) else (
    echo Already in PATH.
)

echo Registering .luelite extension...
reg add "HKEY_CLASSES_ROOT\.luelite" /ve /d "Luelite.Script" /f >nul
reg add "HKEY_CLASSES_ROOT\Luelite.Script" /ve /d "Luelite Script" /f >nul
reg add "HKEY_CLASSES_ROOT\Luelite.Script\DefaultIcon" /ve /d "%INSTALLDIR:\=\\%\\luelite.exe,0" /f >nul
reg add "HKEY_CLASSES_ROOT\Luelite.Script\Shell\Open\Command" /ve /d "\"%INSTALLDIR:\=\\%\\luelite.exe\" \"%%1\"" /f >nul

echo.
echo ========================================
echo   Installation complete!
echo.
echo   Now you can:
echo   - Run .luelite files by double-click
echo   - Type "luelite" in any terminal
echo.
echo   RESTART your terminal to apply PATH.
echo ========================================
pause