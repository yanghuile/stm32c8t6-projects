@echo off
chcp 65001 >nul
echo ����������̲���...
rmdir /s /q Objects 2>nul
rmdir /s /q Listings 2>nul
rmdir /s /q DebugConfig 2>nul
del /f /s /q *.o *.crf *.d *.lst *.htm *.lnp *.dep *.axf *.hex *.map 2>nul
del /f /s /q *.uvguix.* 2>nul
del /f /s /q JLinkLog* JLinkSettings* 2>nul
del /f /s /q Target_1_*.dbgconf 2>nul
echo ������ɣ�
pause