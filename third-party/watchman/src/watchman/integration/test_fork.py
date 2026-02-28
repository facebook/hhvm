# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import os

import pywatchman
from watchman.integration.lib import WatchmanTestCase


@WatchmanTestCase.expand_matrix
class TestSince(WatchmanTestCase.WatchmanTestCase):
    def checkOSApplicability(self) -> None:
        if not getattr(os, "fork", None):
            self.skipTest("no fork on this system")

    def test_forkclient(self) -> None:
        client = self.getClient()

        client.query("version")

        pid = os.fork()
        if pid == 0:
            # I am the new process
            try:
                with self.assertRaises(pywatchman.UseAfterFork) as ctx:
                    client.query("version")
                self.assertIn(
                    "do not re-use a connection after fork", str(ctx.exception)
                )

                # All good
                os._exit(0)
            except BaseException as exc:
                print("Error in child process: %s" % exc)
                os._exit(1)

        _pid, status = os.waitpid(pid, 0)
        self.assertEqual(status, 0, "child process exited 0")
