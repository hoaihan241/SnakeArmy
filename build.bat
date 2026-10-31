@echo off
chcp 65001 > nul
echo ========================================================
echo   🐍 SNAKE ARMY 2.0 - TIẾN HÀNH BIÊN DỊCH TRÒ CHƠI...
echo ========================================================

if not exist bin mkdir bin

echo [*] Đang biên dịch toàn bộ mã nguồn C++17...
g++ -std=c++17 src\*.cpp src\Core\*.cpp src\Entities\*.cpp src\DataStructures\*.cpp src\Items\*.cpp src\Events\*.cpp -o bin\main.exe -I C:\sfml\include -L C:\sfml\lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -mwindows

if %ERRORLEVEL% EQU 0 (
    echo.
    echo [V] BIÊN DỊCH THÀNH CÔNG! File thực thi: bin\main.exe
    echo ========================================================
    echo [*] Đang khởi chạy game...
    start bin\main.exe
) else (
    echo.
    echo [X] BIÊN DỊCH THẤT BẠI! Vui lòng kiểm tra lại MinGW g++ và thư viện SFML.
    pause
)

