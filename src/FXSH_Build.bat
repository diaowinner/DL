@echo off
rem Do not edit! This batch file is created by CASIO fx-9860G SDK.

if exist debug\*.obj  del debug\*.obj
if exist CHREADER.G1A  del CHREADER.G1A

cd debug
if exist FXADDINror.bin  del FXADDINror.bin
"F:\SDKcalc\OS\SH\Bin\Hmake.exe" Addin.mak
cd ..
if not exist debug\FXADDINror.bin  goto error

"F:\SDKcalc\Tools\MakeAddinHeader363.exe" "E:\DLsrc"
if not exist CHREADER.G1A  goto error
echo Build has completed.
goto end

:error
echo Build was not successful.

:end

