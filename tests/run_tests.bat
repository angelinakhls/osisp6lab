@echo off
setlocal
set BIN_DIR=..\cmake-build-debug\bin
if not exist "%BIN_DIR%\lab06_app.exe" (
    set BIN_DIR=..\cmake-build-release\bin
)
if not exist "%BIN_DIR%\lab06_app.exe" (
    echo [ERROR] lab06_app.exe not found. Build the project in CLion first.
    exit /b 1
)

echo === Running all scenarios (Demo + Error Tests + Benchmarks) ===
"%BIN_DIR%\lab06_app.exe" --dll "%BIN_DIR%\KhlusMathLib.dll" --mode all --student-id 114 --runs 3 --iterations 1000 --export ..\data\benchmark_results.csv --log ..\data\lab06_run.log

echo.
echo === Generating charts via Python ===
python ..\scripts\plot_results.py ..\data\benchmark_results.csv ..\diagrams
endlocal
