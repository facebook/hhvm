
@echo off

if "%1" == "" goto build
if "%1" == "build" goto build
if "%1" == "all" goto build
if "%1" == "test" goto test
if "%1" == "init" goto init
if "%1" == "clean" goto clean
REM Invalid argument
echo Invalid argument, please check README.win32
goto end

REM initialize ocp-build and create _obuild directory
:init
if not exist "_obuild/" ocp-build init
goto end

REM 1/ check if ocp-build init has already be done
REM 2/ generate get_build_id.gen.c
REM 3/ start build hack with ocp-build
:build
if not exist "_obuild/" ocp-build init
ocaml.exe unix.cma ./scripts/gen_build_id.ml ./src/utils/get_build_id.gen.c
ocaml.exe unix.cma ./scripts/gen_index.ml hhi.rc hhi
ocp-build
md bin 2>NUL
copy _obuild\hh_server\hh_server.asm.exe bin\hh_server.exe
copy _obuild\hh_client\hh_client.asm.exe bin\hh_client.exe
copy _obuild\hh_single_type_check\hh_single_type_check.asm.exe bin\hh_single_type_check.exe
copy _obuild\hh_format\hh_format.asm.exe bin\hh_format.exe
copy _obuild\h2tp\h2tp.asm.exe bin\h2tp.exe

goto end

REM clean _obuild directory and executables in bin/
:clean
ocp-build clean
del bin\hh_server.exe 2>NUL
del bin\hh_client.exe 2>NUL
del bin\hh_single_type_check.exe 2>NUL
del bin\hh_format.exe 2>NUL
del bin\h2tp.exe 2>NUL
goto end

REM execute the Hack testsuite
:test
@echo on

set "python3=python.exe"
set "max=8"
"%python3%" test\verify.py --max-workers "%max%" --program bin\hh_single_type_check.exe test\autocomplete
"%python3%" test\verify.py --max-workers "%max%" --program bin\hh_single_type_check.exe test\color
"%python3%" test\verify.py --max-workers "%max%" --program bin\hh_single_type_check.exe test\colour
"%python3%" test\verify.py --max-workers "%max%" --program bin\hh_single_type_check.exe test\coverage
"%python3%" test\verify.py --max-workers "%max%" --program bin\hh_single_type_check.exe test\dumpsymbolinfo
"%python3%" test\verify.py --max-workers "%max%" --program bin\hh_format.exe test\format
"%python3%" test\verify.py --max-workers "%max%" --program bin\hh_single_type_check.exe test\suggest
"%python3%" test\verify.py --max-workers "%max%" --program bin\hh_single_type_check.exe test\typecheck
"%python3%" test\verify.py --max-workers "%max%" --program bin\hh_format.exe test\typecheck ^
		--disabled-extension .no_format ^
		--out-extension .format_out ^
		--expect-extension "" ^
		--flags --root .
:end
