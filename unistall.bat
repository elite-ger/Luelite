@echo off
echo Uninstalling Luelite...

reg delete "HKEY_CLASSES_ROOT\.luelite" /f
reg delete "HKEY_CLASSES_ROOT\Luelite.Script" /f

del /Q "%USERPROFILE%\retrelite\luelite.exe"

echo Done.
pause