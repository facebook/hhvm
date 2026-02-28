# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe

import os
import os.path
import signal
import subprocess
import tempfile
import unittest

from watchman.integration.lib import Interrupt, WatchmanInstance

WATCHMAN_SRC_DIR: str = os.environ.get("WATCHMAN_SRC_DIR", os.getcwd())
TEST_BINARY = (
    os.environ["WATCHMAN_CPPCLIENT_BINARY"]
    if "WATCHMAN_CPPCLIENT_BINARY" in os.environ.keys()
    else os.path.join(WATCHMAN_SRC_DIR, "integration/cppclient.t")
)


@unittest.skipIf(os.name == "nt", "cppclient doesn't run on Windows")
class TestCppClient(unittest.TestCase):
    def setUp(self) -> None:
        self.tmpDirCtx = tempfile.TemporaryDirectory()  # noqa P201
        self.tmpDir = self.tmpDirCtx.__enter__()

    def tearDown(self) -> None:
        self.tmpDirCtx.__exit__(None, None, None)

    @unittest.skipIf(not os.path.isfile(TEST_BINARY), "test binary not built")
    def test_cppclient(self) -> None:
        env = os.environ.copy()
        env["WATCHMAN_SOCK"] = (
            WatchmanInstance.getSharedInstance().getSockPath().legacy_sockpath()
        )
        proc = subprocess.Popen(
            TEST_BINARY,
            env=env,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            cwd=self.tmpDir,
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

        self.assertTrue(True, TEST_BINARY)
