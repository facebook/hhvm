# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import os
import shutil

from watchman.integration.lib import WatchmanTestCase


@WatchmanTestCase.expand_matrix
class TestRemove(WatchmanTestCase.WatchmanTestCase):
    def test_remove(self) -> None:
        root = self.mkdtemp()
        os.makedirs(os.path.join(root, "one", "two"))
        self.touchRelative(root, "one", "onefile")
        self.touchRelative(root, "one", "two", "twofile")
        self.touchRelative(root, "top")

        self.watchmanCommand("watch", root)
        self.assertFileList(
            root, files=["one", "one/onefile", "one/two", "one/two/twofile", "top"]
        )

        shutil.rmtree(os.path.join(root, "one"))

        self.assertFileList(root, files=["top"])

        self.touchRelative(root, "one")
        self.assertFileList(root, files=["top", "one"])

        self.removeRelative(root, "one")
        self.assertFileList(root, files=["top"])

        shutil.rmtree(root)
        os.makedirs(os.path.join(root, "notme"))

        self.assertWaitFor(
            lambda: not self.rootIsWatched(root),
            message="%s should be cancelled" % root,
        )
