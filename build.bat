@echo off
setlocal
cd /d "%~dp0"

echo ========================================================
echo   SNAKE ARMY 2.0 - TIEN HANH BIEN DICH TRO CHOI...
echo ========================================================

if not exist bin mkdir bin

echo [*] Dang bien dich toan bo ma nguon C++17...
if exist "C:\sfml\include" (
    g++ -std=c++17 src\*.cpp src\Core\*.cpp src\Entities\*.cpp src\DataStructures\*.cpp src\Items\*.cpp src\Events\*.cpp -o bin\main.exe -I C:\sfml\include -L C:\sfml\lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -mwindows
) else (
    g++ -std=c++17 src\*.cpp src\Core\*.cpp src\Entities\*.cpp src\DataStructures\*.cpp src\Items\*.cpp src\Events\*.cpp -o bin\main.exe -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -mwindows
)

if %ERRORLEVEL% EQU 0 (
    echo.
    echo [V] BIEN DICH THANH CONG! File thuc thi: bin\main.exe
    echo ========================================================
    echo [*] Dang khoi chay game...
    start "" "bin\main.exe"
) else (
    echo.
    echo [X] BIEN DICH THAT BAI! Vui long kiem tra MinGW g++ va SFML.
    pause
)
