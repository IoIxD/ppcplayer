@ECHO OFF
REM IF EXIST "%~dp0\build" rmdir "%~dp0\build" /S /Q
REM md build
IF NOT EXIST "%~dp0\build" md build
cd build
cmake .. -G "Visual Studio 14" -DANTEXE=D:\Code\ant\bin\ant
cd ..
