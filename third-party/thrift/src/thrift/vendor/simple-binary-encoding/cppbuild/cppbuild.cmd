@if "%DEBUG%" == "" @echo off
setlocal EnableDelayedExpansion

set SOURCE_DIR=%CD%
set BUILD_DIR=%CD%\cppbuild\Release

for %%o in (%*) do (

    set PROCESSED=0

    if "%%o"=="--help" (
        echo cppbuild.cmd [--c-warnings-as-errors] [--cxx-warnings-as-errors]
        exit /b
    )

    if "%%o"=="--c-warnings-as-errors" (
        set EXTRA_CMAKE_ARGS=!EXTRA_CMAKE_ARGS! -DC_WARNINGS_AS_ERRORS=ON
        set PROCESSED=1
    )

    if "%%o"=="--cxx-warnings-as-errors" (
        set EXTRA_CMAKE_ARGS=!EXTRA_CMAKE_ARGS! -DCXX_WARNINGS_AS_ERRORS=ON
        set PROCESSED=1
    )
)

call cppbuild/vs-helper.cmd
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%

if EXIST %BUILD_DIR% rd /S /Q %BUILD_DIR%

md %BUILD_DIR%
pushd %BUILD_DIR%

cmake %EXTRA_CMAKE_ARGS% %SOURCE_DIR%
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%

cmake --build . --clean-first --config Release
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%

ctest -C Release --output-on-failure
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%
