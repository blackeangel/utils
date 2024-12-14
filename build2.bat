@echo off
setlocal enabledelayedexpansion
set PATH=%PATH%;"C:\Program Files\JetBrains\CLion 2024.1\bin\ninja\win\x64";"C:\Program Files\JetBrains\CLion 2024.1\bin\cmake\win\x64\bin"
rem Определяем переменные
set path_ndk="C:\android-ndk"

set ROOT_DIR=%cd%
set DIST_DIR=%ROOT_DIR%\dist
set BUILD_DIR=%ROOT_DIR%\build

REM set ABIS=armeabi-v7a arm64-v8a
set ABIS=armeabi-v7a
set ANDROID_NDK=%path_ndk%

rem Основной цикл сборки
for %%A in (%ABIS%) do (
    set arch=%%A
    if exist "%BUILD_DIR%" rd /s /q "%BUILD_DIR%"
    mkdir "%BUILD_DIR%"
    cd "%BUILD_DIR%"
    cmake.exe -DANDROID_ABI=!arch! ^
        -DCMAKE_TOOLCHAIN_FILE=%ANDROID_NDK%\build\cmake\android.toolchain.cmake ^
        -DANDROID_NATIVE_API_LEVEL=30 ^
        -GNinja ..
    ninja.exe
    if not exist "%DIST_DIR%\!arch!" mkdir "%DIST_DIR%\!arch!"
    copy "%BUILD_DIR%\bin_utils" "%DIST_DIR%\!arch%\" /y
    %ANDROID_NDK%\toolchains\llvm\prebuilt\windows-x86_64\bin\llvm-strip.exe -s "%DIST_DIR%\!arch%\bin_utils"
    cd "%ROOT_DIR%"
)

cd %ROOT_DIR%
