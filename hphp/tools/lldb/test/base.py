# Copyright 2022-present Facebook. All Rights Reserved.

import os
import pathlib
import sys
import subprocess
from libfb.py.testutil import BaseFacebookTestCase

# For lldb, until there's a buck-visible library we can use:
lldb_path = subprocess.run(["/opt/llvm/bin/lldb", "-P"], stdout=subprocess.PIPE).stdout.decode("utf-8").strip()
sys.path.append(lldb_path)
import lldb
from fblldb.utils import run_lldb_command, get_lldb_object_description

hhvm_path = pathlib.Path(__file__).parent.parent / "hhvm_cpp"
hhvm_test_path = os.environ["HHVM_TEST_DIR"]
scripts_path = os.environ["HHVM_LLDB_SCRIPTS"]

class LLDBTestBase(BaseFacebookTestCase):
    def setUp(self, file=None):
        debugger = lldb.SBDebugger.Create()
        debugger.SetAsync(False)  # Important so that we only stop at our breakpoint, not arbitrary signals
        assert debugger.IsValid(), "Unable to create debugger instance"

        target = debugger.CreateTarget(hhvm_path.as_posix())
        assert target.IsValid(), "Unable to create target"

        error = lldb.SBError()
        process = target.Launch(
            debugger.GetListener(),
            ["--file", f"{hhvm_test_path}/{file}"] if file else None,
            None, None, None, None, os.getcwd(), 0, True, error)
        assert process.IsValid() and error.Success(), f"Unable to launch process ({get_lldb_object_description(error)})"
        assert process.GetState() == lldb.eStateStopped, "Process is not in Stopped state"

        (status, output) = run_lldb_command(debugger, f"command script import {scripts_path}/hhvm.py")
        assert status == 0, output  # output will be the error message

        self.debugger = debugger

    def tearDown(self):
        lldb.SBDebugger.Destroy(self.debugger)
        self.debugger = None

    def getProcess(self):
        return self.debugger.GetSelectedTarget().GetProcess()

    def run_command(self, command: str) -> str:
        (status, output) = run_lldb_command(self.debugger, command)
        self.assertEqual(status, 0, output)
        return output

    def run_until_breakpoint(self, breakpoint: str):
        breakpoint = self.debugger.GetSelectedTarget().BreakpointCreateByName(breakpoint)
        self.assertTrue(breakpoint.IsValid(), f"Unable to set breakpoint at {breakpoint}")
        process = self.getProcess()
        err = process.Continue()
        assert err.Success(), f"Unable to reach breakpoint at {breakpoint}"
        assert process.GetSelectedThread().GetStopReason() == lldb.eStopReasonBreakpoint
