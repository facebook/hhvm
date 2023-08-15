# Copyright 2022-present Facebook. All Rights Reserved.

import os
import pathlib
import sys
import subprocess
import typing
import unittest
from libfb.py.testutil import BaseFacebookTestCase
from __manifest__ import fbmake as build_info

# For lldb, until there's a buck-visible library we can use.
# Its path on TW containers starts at /host-mounts.
lldb_path = None
for path in ['/opt/llvm/bin/lldb', '/host-mounts/opt/llvm/bin/lldb']:
    if os.path.exists(path):
        lldb_path = subprocess.run([path, "-P"], stdout=subprocess.PIPE).stdout.decode("utf-8").strip()
        break

assert lldb_path, "Couldn't find lldb on host"

sys.path.append(lldb_path)
import lldb
from fblldb.utils import run_lldb_command, get_lldb_object_description

hhvm_path = pathlib.Path(__file__).parent.parent / "hhvm"
hhvm_types_path = pathlib.Path(__file__).parent.parent / "hhvm_types_for_lldb_tests"
hhvm_test_path = os.environ["HHVM_TEST_DIR"]
scripts_path = os.environ["HHVM_LLDB_SCRIPTS"]

@unittest.skipIf(build_info["build_mode"].startswith("opt"), "Need non-opt build for debugging")
class LLDBTestBase(BaseFacebookTestCase):
    def setUp(self):
        """ Set up a debugger for each test case instance """
        debugger = lldb.SBDebugger.Create()
        debugger.SetAsync(False)  # Important so that we only stop at our breakpoint, not arbitrary signals
        assert debugger.IsValid(), "Unable to create debugger instance"

        target = debugger.CreateTarget(self.getTargetPath())
        assert target.IsValid(), "Unable to create target"

        if self.launchProcess():
            error = lldb.SBError()
            process = target.Launch(
                debugger.GetListener(),
                self.getArgs(),
                None, None, None, None, os.getcwd(), 0, True, error)
            assert process.IsValid() and error.Success(), f"Unable to launch process ({get_lldb_object_description(error)})"
            assert process.GetState() == lldb.eStateStopped, "Process is not in Stopped state"

        (status, output) = run_lldb_command(debugger, f"command script import {scripts_path}/hhvm.py")
        assert status == 0, output  # output will be the error message

        self.debugger = debugger

    def launchProcess(self) -> bool:
        return True

    @typing.abstractmethod
    def getTargetPath(self) -> str:
        pass

    def getArgs(self) -> typing.List[str]:
        return []

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
        process = self.target.GetProcess()
        assert process.IsValid(), f"Invalid process; self.launchProcess() returns '{self.launchProcess()}'"
        return process

    @property
    def thread(self) -> lldb.SBThread:
        """ Get the process associated wih the debugger's target """
        return self.process.GetSelectedThread()

    @property
    def frame(self) -> lldb.SBProcess:
        """ Get the topmost frame associated with the current thread """
        return self.thread.GetFrameAtIndex(0)

    def run_commands(self, commands: typing.List[str], check: bool = True) -> typing.Tuple[int, str]:
        """ Run one or more LLDB commands in the interpreter.

        Arguments:
            commands: List of commands (strings) to run
            check: If True (default), check that each command succeeds before running the next

        Returns:
            A (status, output) tuple corresponding to the status of the last command ran
        """
        for command in commands:
            (status, output) = run_lldb_command(self.debugger, command)
            if check:
                self.assertEqual(status, 0, output)
        return (status, output)

    def run_until_breakpoint(self, breakpoint: str):
        """ Run until the breakpoint given by a function name is hit """
        breakpoint = self.debugger.GetSelectedTarget().BreakpointCreateByName(breakpoint)
        self.assertTrue(breakpoint.IsValid(), f"Unable to set breakpoint at {breakpoint}")
        err = self.process.Continue()
        assert err.Success(), f"Unable to continue to breakpoint {breakpoint}"
        assert self.process.GetSelectedThread().GetStopReason() == lldb.eStopReasonBreakpoint, f"Unable to reach breakpoint at {breakpoint}"

class TestHHVMBinary(LLDBTestBase):
    def setUp(self, launch_process=True, test_file=None, interp=False, allow_hhas=False):
        self.launch_process = launch_process
        self.test_file = test_file
        self.interp = interp
        self.allow_hhas = allow_hhas
        super().setUp()

    def launchProcess(self) -> bool:
        return self.launch_process

    def getTargetPath(self) -> str:
        return hhvm_path.as_posix()

    def getArgs(self) -> typing.List[str]:
        hhvm_args = []
        if self.test_file:
            hhvm_args.extend(["--file", f"{hhvm_test_path}/{self.test_file}"])
        if self.interp:
            hhvm_args.extend(["-vEval.Jit=0"])
        if self.allow_hhas:
            hhvm_args.extend(["-vEval.AllowHhas=true"])
        return hhvm_args

class TestHHVMTypesBinary(LLDBTestBase):

    def setUp(self, test_type: str):
        self.test_type = test_type
        super().setUp()

    def getTargetPath(self) -> str:
        return hhvm_types_path.as_posix()

    def getArgs(self) -> typing.List[str]:
        return [self.test_type]
