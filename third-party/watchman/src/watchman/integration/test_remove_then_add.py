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
class TestRemoveThenAdd(WatchmanTestCase.WatchmanTestCase):
    def checkOSApplicability(self) -> None:
        if os.name == "linux" and os.getenv("TRAVIS"):
            self.skipTest("openvz and inotify unlinks == bad time")

    def test_remove_then_add(self) -> None:
        root = self.mkdtemp()
        os.mkdir(os.path.join(root, "foo"))

        self.watchmanCommand("watch", root)

        self.touchRelative(root, "foo", "222")
        os.mkdir(os.path.join(root, "foo", "bar"))

        self.assertFileList(root, files=["foo", "foo/bar", "foo/222"])

        shutil.rmtree(os.path.join(root, "foo", "bar"))
        self.removeRelative(root, "foo", "222")
        shutil.rmtree(os.path.join(root, "foo"))

        self.assertFileList(root, files=[])

        os.mkdir(os.path.join(root, "foo"))
        self.touchRelative(root, "foo", "222")

        self.assertFileList(root, files=["foo", "foo/222"])
