@echo off

cd ..

echo Generating project files
cmake -S . -B magnet/Build -G "Visual Studio 17 2022" -A x64 -DCMAKE_CXX_COMPILER=clang++

echo Building project
cmake --build magnet/Build --config Debug

echo Setting up environment variables
rem Directory containing the magnet executable
set "MAGNET_DIR=%~dp0..\magnet\Binaries\Debug"

rem Checking if Magnet is already in the PATH
echo %PATH% | find /i "%MAGNET_DIR%" > nul
if errorlevel 1 (
  rem Add the magnet directory to the PATH
  setx PATH "%PATH%;%MAGNET_DIR%" /M
  echo Added magnet to the PATH
)

echo Launching Visual Studio Solution
start %~dp0..\magnet\Build\magnet.sln

PAUSE
