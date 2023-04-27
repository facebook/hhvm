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
setlocal
for %%D in ("%tbb_root%") do set actual_root=%%~fD
set fslash_root=%actual_root:\=/%
set bin_dir=%CD%
set fslash_bin_dir=%bin_dir:\=/%
set _INCLUDE=INCLUDE& set _LIB=LIB
if not x%UNIXMODE%==x set _INCLUDE=CPATH& set _LIB=LIBRARY_PATH
if not x%USE_INCLUDE_ENV%==x set _INCLUDE=INCLUDE

echo Generating local tbbvars.bat
echo @echo off>tbbvars.bat
echo SET TBBROOT=%actual_root%>>tbbvars.bat
echo SET TBB_ARCH_PLATFORM=%arch%\%runtime%>>tbbvars.bat
echo SET TBB_TARGET_ARCH=%arch%>>tbbvars.bat
echo SET %_INCLUDE%=%%TBBROOT%%\include;%%%_INCLUDE%%%>>tbbvars.bat
echo SET %_LIB%=%bin_dir%;%%%_LIB%%%>>tbbvars.bat
echo SET PATH=%bin_dir%;%%PATH%%>>tbbvars.bat
if not x%UNIXMODE%==x echo SET LD_LIBRARY_PATH=%bin_dir%;%%LD_LIBRARY_PATH%%>>tbbvars.bat

echo Generating local tbbvars.sh
echo #!/bin/sh>tbbvars.sh
echo export TBBROOT="%fslash_root%">>tbbvars.sh
echo export TBB_ARCH_PLATFORM="%arch%\%runtime%">>tbbvars.sh
echo export TBB_TARGET_ARCH="%arch%">>tbbvars.sh
echo export %_INCLUDE%="${TBBROOT}/include;$%_INCLUDE%">>tbbvars.sh
echo export %_LIB%="%fslash_bin_dir%;$%_LIB%">>tbbvars.sh
echo export PATH="%fslash_bin_dir%;$PATH">>tbbvars.sh
if not x%UNIXMODE%==x echo export LD_LIBRARY_PATH="%fslash_bin_dir%;$LD_LIBRARY_PATH">>tbbvars.sh

echo Generating local tbbvars.csh
echo #!/bin/csh>tbbvars.csh
echo setenv TBBROOT "%actual_root%">>tbbvars.csh
echo setenv TBB_ARCH_PLATFORM "%arch%\%runtime%">>tbbvars.csh
echo setenv TBB_TARGET_ARCH "%arch%">>tbbvars.csh
echo setenv %_INCLUDE% "${TBBROOT}\include;$%_INCLUDE%">>tbbvars.csh
echo setenv %_LIB% "%bin_dir%;$%_LIB%">>tbbvars.csh
echo setenv PATH "%bin_dir%;$PATH">>tbbvars.csh
if not x%UNIXMODE%==x echo setenv LD_LIBRARY_PATH "%bin_dir%;$LD_LIBRARY_PATH">>tbbvars.csh

if not x%LIB_STL_ANDROID%==x (
REM Workaround for copying Android* specific stl shared library to work folder
copy /Y "%LIB_STL_ANDROID:/=\%" .
)

endlocal
exit
