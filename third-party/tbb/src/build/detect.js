// Copyright (c) 2005-2018 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//
//
//

function readAllFromFile(fname) {
    var fso = new ActiveXObject("Scripting.FileSystemObject");
    var file = null;
    try {
        file = fso.OpenTextFile(fname, 1, 0);
        return (file.readAll());
    } finally {
        // Close the file in the finally section to guarantee that it will be closed in any case
        // (if the exception is thrown or not).
        file.Close();
    }
}

function doWork() {
    var WshShell = WScript.CreateObject("WScript.Shell");

    var tmpExec = WshShell.Run("cmd /c echo int main(){return 0;} >detect.c", 0, true);

    // The next block deals with GCC (MinGW)
    if (WScript.Arguments.Count() > 1) {
        var compilerPath = WScript.Arguments(1);
        // The RegExp matches everything up to and including the last slash (it uses a greedy approach.)
        var compilerName = compilerPath.replace(/^.*[\/\\]/, "");
        if (compilerName.match(/gcc/i) != null) {
            if (WScript.Arguments(0) == "/arch") {
                // Get predefined macros
                tmpExec = WshShell.Run("cmd /C " + compilerPath + " -dM -E detect.c > detect.map", 0, true);
                var defs = readAllFromFile("detect.map");
                //detect target architecture
                var intel64 = /x86_64|amd64/mgi;
                var ia32 = /i386/mgi;
                if (defs.match(intel64)) {
                    WScript.Echo("intel64");
                } else if (defs.match(ia32)) {
                    WScript.Echo("ia32");
                } else {
                    WScript.Echo("unknown");
                }
            } else {
                tmpExec = WshShell.Exec(compilerPath + " -dumpversion");
                var gccVersion = tmpExec.StdOut.ReadLine();
                if (WScript.Arguments(0) == "/runtime") {
                    WScript.Echo("mingw" + gccVersion);
                }
                else if (WScript.Arguments(0) == "/minversion") {
                    // Comparing strings, not numbers; will not work for two-digit versions
                    if (gccVersion >= WScript.Arguments(2)) {
                        WScript.Echo("ok");
                    } else {
                        WScript.Echo("fail");
                    }
                }
            }
            return;
        }
    }

    //Compile binary
    tmpExec = WshShell.Exec("cl /MD detect.c /link /MAP");
    while (tmpExec.Status == 0) {
        WScript.Sleep(100);
    }
    //compiler banner that includes version and target arch was printed to stderr
    var clVersion = tmpExec.StdErr.ReadAll();

    if (WScript.Arguments(0) == "/arch") {
        //detect target architecture
        var intel64 = /AMD64|EM64T|x64/mgi;
        var ia32 = /[80|\s]x86/mgi;
        var arm = /ARM/mgi;
        if (clVersion.match(intel64)) {
            WScript.Echo("intel64");
        } else if (clVersion.match(ia32)) {
            WScript.Echo("ia32");
        } else if (clVersion.match(arm)) {
            WScript.Echo("armv7");
        } else {
            WScript.Echo("unknown");
        }
        return;
    }

    if (WScript.Arguments(0) == "/runtime") {
        //read map-file
        var mapContext = readAllFromFile("detect.map");
        //detect runtime
        var vc71 = /MSVCR71\.DLL/mgi;
        var vc80 = /MSVCR80\.DLL/mgi;
        var vc90 = /MSVCR90\.DLL/mgi;
        var vc100 = /MSVCR100\.DLL/mgi;
        var vc110 = /MSVCR110\.DLL/mgi;
        var vc120 = /MSVCR120\.DLL/mgi;
        var vc140 = /VCRUNTIME140\.DLL/mgi;
        var psdk = /MSVCRT\.DLL/mgi;
        if (mapContext.match(vc71)) {
            WScript.Echo("vc7.1");
        } else if (mapContext.match(vc80)) {
            WScript.Echo("vc8");
        } else if (mapContext.match(vc90)) {
            WScript.Echo("vc9");
        } else if (mapContext.match(vc100)) {
            WScript.Echo("vc10");
        } else if (mapContext.match(vc110)) {
            WScript.Echo("vc11");
        } else if (mapContext.match(vc120)) {
            WScript.Echo("vc12");
        } else if (mapContext.match(vc140)) {
            if (WshShell.ExpandEnvironmentStrings("%VisualStudioVersion%") == "15.0")
                WScript.Echo("vc14.1");
            else
                WScript.Echo("vc14");
        } else {
            WScript.Echo("unknown");
        }
        return;
    }

    if (WScript.Arguments(0) == "/minversion") {
        var compilerVersion;
        var compilerUpdate;
        if (WScript.Arguments(1) == "cl") {
            compilerVersion = clVersion.match(/Compiler Version ([0-9.]+)\s/mi)[1];
            // compilerVersion is in xx.xx.xxxxx.xx format, i.e. a string.
            // It will compare well with major.minor versions where major has two digits,
            // which is sufficient as the versions of interest start from 13 (for VC7).
        } else if (WScript.Arguments(1) == "icl") {
            // Get predefined ICL macros
            tmpExec = WshShell.Run("cmd /C icl /QdM /E detect.c > detect.map", 0, true);
            var defs = readAllFromFile("detect.map");
            // In #define __INTEL_COMPILER XXYY, XX is the major ICL version, YY is minor
            compilerVersion = defs.match(/__INTEL_COMPILER[ \t]*([0-9]+).*$/mi)[1] / 100;
            compilerUpdate = defs.match(/__INTEL_COMPILER_UPDATE[ \t]*([0-9]+).*$/mi)[1];
            // compiler version is a number; it compares well with another major.minor
            // version number, where major has one, two, and perhaps more digits (9.1, 11, etc).
        }
        var requestedVersion = WScript.Arguments(2);
        var requestedUpdate = 0;
        if (WScript.Arguments.Count() > 3)
            requestedUpdate = WScript.Arguments(3);
        if (compilerVersion < requestedVersion) {
            WScript.Echo("fail");
        } else if (compilerVersion == requestedVersion && compilerUpdate < requestedUpdate) {
            WScript.Echo("fail");
        } else {
            WScript.Echo("ok");
        }
        return;
    }
}

function doClean() {
    var fso = new ActiveXObject("Scripting.FileSystemObject");
    // delete intermediate files
    if (fso.FileExists("detect.c"))
        fso.DeleteFile("detect.c", false);
    if (fso.FileExists("detect.obj"))
        fso.DeleteFile("detect.obj", false);
    if (fso.FileExists("detect.map"))
        fso.DeleteFile("detect.map", false);
    if (fso.FileExists("detect.exe"))
        fso.DeleteFile("detect.exe", false);
    if (fso.FileExists("detect.exe.manifest"))
        fso.DeleteFile("detect.exe.manifest", false);
}

if (WScript.Arguments.Count() > 0) {

    try {
        doWork();
    } catch (error) {
        WScript.Echo("unknown");
    }
    doClean();

} else {
    WScript.Echo("Supported options:\n"
                  + "\t/arch [compiler]\n"
                  + "\t/runtime [compiler]\n"
                  + "\t/minversion compiler version");
}

