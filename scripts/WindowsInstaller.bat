cd ..

echo Generating project files

cmake -S . -B magnet/build -G "Visual Studio 17 2022" -A x64 -DCMAKE_CXX_COMPILER=clang++

echo Building project
cmake --build magnet/build --config Debug

setlocal

echo Setting up environment variables
rem Directory containing the magnet executable
set MAGNET_DIR=%~dp0..\magnet\Binaries\Debug

rem Add the magnet directory to the PATH
set PATH=%PATH%;%MAGNET_DIR%

echo Directory added to PATH: %MAGNET_DIR%
echo New PATH: %PATH%

endlocal

PAUSE