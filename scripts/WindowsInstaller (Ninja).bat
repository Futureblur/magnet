@echo off

cd ..

rem Remove old build files if they exist
if exist magnet/Build (
  echo "Removing old build files"
  rmdir /s /q "magnet/Build"
)

rem Remove old binaries if they exist
if exist magnet/Binaries (
  echo "Removing old binaries"
  rmdir /s /q "magnet/Binaries"
)

echo Running Ninja generator
cmake -S . -B magnet/Build -G Ninja -DCMAKE_BUILD_TYPE=Debug

echo Building project
cmake --build magnet/Build

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
