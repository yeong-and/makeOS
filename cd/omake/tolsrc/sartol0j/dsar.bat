@echo off

set dsar_sartol=sartol.exe
set dsar_bpath="%USERPROFILE%/����ũž/"
set dsar_autorun="%SystemRoot%\explorer.exe"

rem ����-
rem set dsar_bpath="%USERPROFILE%/����ũž/"
rem set dsar_bpath="%USERPROFILE%/����ũž"
rem set dsar_bpath=..@arcpath/
rem set dsar_bpath=..@arcpath

rem ����-
rem set dsar_autorun="%SystemRoot%\explorer.exe"
rem set dsar_autorun=
rem ����! dsar_bpath������ �н��� �ܶ��� \�� ����� ��

:loop
if %1. ==.  goto end
%dsar_sartol% d %1 %dsar_bpath% %dsar_autorun%
shift
goto loop
:end