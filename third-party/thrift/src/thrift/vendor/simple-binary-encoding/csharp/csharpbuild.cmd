@ECHO OFF
SETLOCAL

SET VERSION=1.0.7.9
SET MSBUILD="c:/Program Files (x86)/Microsoft Visual Studio/2017/Community/MSBuild/15.0/Bin/msbuild.exe"
SET MSTEST="c:/Program Files (x86)/Microsoft Visual Studio/2017/Community/Common7/IDE/MSTest.exe"
SET COPYRIGHT="Copyright (C) Bill Segall 2018, MarketFactory Inc 2017, Adaptive 2014. All rights reserved."
SET CONFIGURATION="Release"

REM Restore packages
%MSBUILD% csharp.sln /target:Restore /property:Configuration=%CONFIGURATION%

REM Rebuild
%MSBUILD% csharp.sln /target:Rebuild /property:Configuration=%CONFIGURATION%

REM Run Tests
%MSTEST% /testcontainer:sbe-tests/bin/%CONFIGURATION%/net45/Org.SbeTool.Sbe.UnitTests.dll

ENDLOCAL
