# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import os
import os.path
import sys

from watchman.integration.lib import WatchmanTestCase


@WatchmanTestCase.expand_matrix
class TestKQueueAndFSEventsRecrawl(WatchmanTestCase.WatchmanTestCase):
    def checkOSApplicability(self) -> None:
        if sys.platform != "darwin":
            self.skipTest("N/A unless macOS")

    def test_recrawl(self) -> None:
        root = self.mkdtemp()
        watch = self.watchmanCommand("watch", root)

        # On macOS, we may not always use kqueue+fsevents
        if watch["watcher"] != "kqueue+fsevents":
            return

        os.mkdir(os.path.join(root, "foo"))
        filelist = ["foo"]

        self.assertFileList(root, filelist)

        self.suspendWatchman()

        filelist = ["foo"]
        for i in range(3000):
            self.touchRelative(root, "foo", str(i))
            filelist.append(f"foo/{i}")

        self.resumeWatchman()

        self.watchmanCommand(
            "debug-kqueue-and-fsevents-recrawl", root, os.path.join(root, "foo")
        )

        self.assertFileList(root, filelist)
