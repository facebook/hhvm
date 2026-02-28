# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import os

from watchman.integration.lib import WatchmanTestCase


@WatchmanTestCase.expand_matrix
class TestWatchDelAll(WatchmanTestCase.WatchmanTestCase):
    def test_watch_del_all(self) -> None:
        root = self.mkdtemp()

        dirs = [os.path.join(root, f) for f in ["a", "b", "c", "d"]]

        for d in dirs:
            os.mkdir(d)
            self.touchRelative(d, "foo")
            self.watchmanCommand("watch", d)
            self.assertFileList(d, files=["foo"])

        self.watchmanCommand("watch-del-all")
        self.assertEqual(self.watchmanCommand("watch-list")["roots"], [])
