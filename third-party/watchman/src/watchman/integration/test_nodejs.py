# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import distutils.spawn
import glob
import inspect
import os
import os.path
import re
import shutil
import signal
import subprocess
import unittest

from watchman.integration.lib import Interrupt, WatchmanInstance, WatchmanTestCase
from watchman.integration.lib.node import node_bin, yarn_bin


WATCHMAN_SRC_DIR: str = os.environ.get("WATCHMAN_SRC_DIR", os.getcwd())
THIS_DIR = os.path.join(WATCHMAN_SRC_DIR, "integration")


def find_js_tests(test_class) -> None:
    """
    A decorator function used to create a class per JavaScript test script
    """

    # We do some rather hacky things here to define new test class types
    # in our caller's scope.  This is needed so that the unittest TestLoader
    # will find the subclasses we define.
    # pyre-fixme[16]: Optional type has no attribute `f_back`.
    caller_scope = inspect.currentframe().f_back.f_locals

    for js in glob.glob(os.path.join(THIS_DIR, "*.js")):
        base = os.path.basename(js)
        if base.startswith(".") or base.startswith("_"):
            continue

        subclass_name = base.replace(".", "_").replace("-", "_")

        def make_class(jsfile):
            # Define a new class that derives from the input class.
            # This has to be a function otherwise jsfile captures
            # the value from the last iteration of the glob loop.

            class JSTest(test_class):
                def getCommandArgs(self):
                    return [node_bin, jsfile]

            # Set the name and module information on our new subclass
            JSTest.__name__ = subclass_name
            JSTest.__qualname__ = subclass_name
            JSTest.__module__ = test_class.__module__

            caller_scope[subclass_name] = JSTest

        make_class(js)

    return None


@find_js_tests
class NodeTestCase(WatchmanTestCase.TempDirPerTestMixin, unittest.TestCase):
    attempt = 0

    def setAttemptNumber(self, attempt: int) -> None:
        """enable flaky test retry"""
        self.attempt = attempt

    @unittest.skipIf(
        yarn_bin is None or node_bin is None, "yarn/node not correctly installed"
    )
    def runTest(self) -> None:
        env = os.environ.copy()
        env["WATCHMAN_SOCK"] = (
            WatchmanInstance.getSharedInstance().getSockPath().legacy_sockpath()
        )
        env["TMPDIR"] = self.tempdir

        # build the node module with yarn
        node_dir = os.path.join(env["TMPDIR"], "fb-watchman")
        shutil.copytree(os.path.join(WATCHMAN_SRC_DIR, "node"), node_dir)

        install_args = [yarn_bin, "install"]
        if "YARN_OFFLINE" in env:
            install_args.append("--offline")

        bser_dir = os.path.join(node_dir, "bser")
        subprocess.check_call(install_args, cwd=bser_dir, env=env)

        env["TMP"] = env["TMPDIR"]
        env["TEMP"] = env["TMPDIR"]
        env["IN_PYTHON_HARNESS"] = "1"
        env["NODE_PATH"] = "%s:%s" % (node_dir, env["TMPDIR"])
        proc = subprocess.Popen(
            # pyre-fixme[16]: `NodeTestCase` has no attribute `getCommandArgs`.
            self.getCommandArgs(),
            env=env,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        (stdout, stderr) = proc.communicate()
        status = proc.poll()

        if status == -signal.SIGINT:
            Interrupt.setInterrupted()
            self.fail("Interrupted by SIGINT")
            return

        if status != 0:
            self.fail(
                "Exit status %d\n%s\n%s\n"
                % (status, stdout.decode("utf-8"), stderr.decode("utf-8"))
            )
            return
        self.assertTrue(True, self.getCommandArgs())

    def _getTempDirName(self):
        dotted = (
            os.path.normpath(self.id())
            .replace(os.sep, ".")
            .replace("tests.integration.", "")
            .replace(".php", "")
        )
        if self.attempt > 0:
            dotted += "-%d" % self.attempt
        return dotted
