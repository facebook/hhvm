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
    def setUp(self, file=None, interp=False):
        """ Set up a debugger for each test case instance """
        debugger = lldb.SBDebugger.Create()
        debugger.SetAsync(False)  # Important so that we only stop at our breakpoint, not arbitrary signals
        assert debugger.IsValid(), "Unable to create debugger instance"

        target = debugger.CreateTarget(hhvm_path.as_posix())
        assert target.IsValid(), "Unable to create target"

        hhvm_args = []
        if file:
            hhvm_args.extend(["--file", f"{hhvm_test_path}/{file}"])
        if interp:
            hhvm_args.extend(["-vEval.Jit=0"])

        error = lldb.SBError()
        process = target.Launch(
            debugger.GetListener(),
            hhvm_args,
            None, None, None, None, os.getcwd(), 0, True, error)
        assert process.IsValid() and error.Success(), f"Unable to launch process ({get_lldb_object_description(error)})"
        assert process.GetState() == lldb.eStateStopped, "Process is not in Stopped state"

        (status, output) = run_lldb_command(debugger, f"command script import {scripts_path}/hhvm.py")
        assert status == 0, output  # output will be the error message

        self.debugger = debugger

    def tearDown(self):
        """ Clean up the debugger """
        lldb.SBDebugger.Destroy(self.debugger)
        self.debugger = None

    @property
    def target(self) -> lldb.SBTarget:
        """ Get the target associated wih the debugger """
        return self.debugger.GetSelectedTarget()

    @property
    def process(self) -> lldb.SBProcess:
        """ Get the process associated wih the debugger's target """
        return self.target.GetProcess()

    @property
    def thread(self) -> lldb.SBThread:
        """ Get the process associated wih the debugger's target """
        return self.process.GetSelectedThread()

    @property
    def frame(self) -> lldb.SBProcess:
        """ Get the topmost frame associated with the current thread """
        return self.thread.GetFrameAtIndex(0)

    def run_command(self, *commands: str) -> str:
        """ Run one or more LLDB commands in the interpreter """
        for command in commands:
            (status, output) = run_lldb_command(self.debugger, command)
            self.assertEqual(status, 0, output)
        return output

    def run_until_breakpoint(self, breakpoint: str):
        """ Run until the breakpoint given by a function name is hit """
        breakpoint = self.debugger.GetSelectedTarget().BreakpointCreateByName(breakpoint)
        self.assertTrue(breakpoint.IsValid(), f"Unable to set breakpoint at {breakpoint}")
        err = self.process.Continue()
        assert err.Success(), f"Unable to continue to breakpoint {breakpoint}"
        assert self.process.GetSelectedThread().GetStopReason() == lldb.eStopReasonBreakpoint, f"Unable to reach breakpoint at {breakpoint}"
