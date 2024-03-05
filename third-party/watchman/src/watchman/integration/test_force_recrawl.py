# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import os

from watchman.integration.lib import WatchmanTestCase


@WatchmanTestCase.expand_matrix
class TestForceRecrawl(WatchmanTestCase.WatchmanTestCase):
    def test_force_recrawl(self) -> None:
        root = self.mkdtemp()
        self.watchmanCommand("watch", root)

        os.mkdir(os.path.join(root, "foo"))
        filelist = ["foo"]

        self.assertFileList(root, filelist)

        self.suspendWatchman()

        filelist = ["foo"]
        for i in range(20000):
            self.touchRelative(root, "foo", str(i))
            filelist.append(f"foo/{i}")

        self.resumeWatchman()

        self.assertFileList(root, filelist)
