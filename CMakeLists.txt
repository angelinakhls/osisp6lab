cmake_minimum_required(VERSION 3.20)
project(Lab06_KhlusMathDll LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Put DLL and EXE into the same output directory so LoadLibrary("KhlusMathLib.dll") works out of the box
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# Dynamic Library: KhlusMathLib.dll
add_library(KhlusMathLib SHARED
    src/dll/math_dll.h
    src/dll/math_dll.cpp
)

target_compile_definitions(KhlusMathLib PRIVATE KHLUS_MATH_DLL_EXPORTS)

# Avoid "lib" prefix under MinGW so the output filename is strictly KhlusMathLib.dll
set_target_properties(KhlusMathLib PROPERTIES
    PREFIX ""
    OUTPUT_NAME "KhlusMathLib"
)

# Main Application: lab06_app.exe
# Notice: We do NOT link KhlusMathLib here because Variant 28 requires EXPLICIT runtime loading (LoadLibrary/GetProcAddress/FreeLibrary)
add_executable(lab06_app
    src/app/logger.h
    src/app/logger.cpp
    src/app/dll_loader.h
    src/app/dll_loader.cpp
    src/app/main.cpp
)

# Ensure the DLL is built whenever the application is built
add_dependencies(lab06_app KhlusMathLib)

if (MSVC)
    target_compile_options(KhlusMathLib PRIVATE /W4 /utf-8)
    target_compile_options(lab06_app PRIVATE /W4 /utf-8)
else()
    target_compile_options(KhlusMathLib PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(lab06_app PRIVATE -Wall -Wextra -Wpedantic)
endif()
