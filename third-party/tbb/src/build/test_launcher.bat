@echo off
REM
REM Copyright (c) 2005-2018 Intel Corporation
REM
REM Licensed under the Apache License, Version 2.0 (the "License");
REM you may not use this file except in compliance with the License.
REM You may obtain a copy of the License at
REM
REM     http://www.apache.org/licenses/LICENSE-2.0
REM
REM Unless required by applicable law or agreed to in writing, software
REM distributed under the License is distributed on an "AS IS" BASIS,
REM WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
REM See the License for the specific language governing permissions and
REM limitations under the License.
REM
REM
REM
REM
REM

set cmd_line=
if DEFINED run_prefix set cmd_line=%run_prefix%
:while
if NOT "%1"=="" (
    REM Verbose mode
    if "%1"=="-v" (
        set verbose=yes
        GOTO continue
    )
    REM Silent mode of 'make' requires additional support for associating
    REM of test output with the test name. Verbose mode is the simplest way
    if "%1"=="-q" (
        set verbose=yes
        GOTO continue
    )
    REM Run in stress mode
    if "%1"=="-s" (
        echo Doing stress testing. Press Ctrl-C to terminate
        set stress=yes
        GOTO continue
    )
    REM Repeat execution specified number of times
    if "%1"=="-r" (
        set repeat=%2
        SHIFT
        GOTO continue
    )
    REM no LD_PRELOAD under Windows
    REM but run the test to check "#pragma comment" construction
    if "%1"=="-l" (
        REM The command line may specify -l with empty dll name,
        REM e.g. "test_launcher.bat -l  app.exe". If the dll name is
        REM empty then %2 contains the application name and the SHIFT
        REM operation is not necessary.
        if exist "%3" SHIFT
        GOTO continue
    )
    REM no need to setup up stack size under Windows
    if "%1"=="-u" GOTO continue
    set cmd_line=%cmd_line% %1
:continue
    SHIFT
    GOTO while
)
set cmd_line=%cmd_line:./=.\%
if DEFINED verbose echo Running %cmd_line%
if DEFINED stress set cmd_line=%cmd_line% ^& IF NOT ERRORLEVEL 1 GOTO stress
:stress
if DEFINED repeat (
    for /L %%i in (1,1,%repeat%) do echo %%i of %repeat%: & %cmd_line%
) else (
    %cmd_line%
)
