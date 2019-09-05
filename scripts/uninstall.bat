@echo off

taskkill /F /IM mcb.exe
del /F "%APPDATA%\Microsoft\Windows\Start Menu\Programs\Startup\mcb.exe"

echo Uninstall completed.
echo Thank you for using this software!
pause
