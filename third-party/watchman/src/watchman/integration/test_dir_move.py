# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import os
import os.path
import shutil
import time

from watchman.integration.lib import WatchmanTestCase


@WatchmanTestCase.expand_matrix
class TestDirMove(WatchmanTestCase.WatchmanTestCase):

    # testing this is flaky at best on windows due to latency
    # and exclusivity of file handles, so skip it.
    def checkOSApplicability(self) -> None:
        if os.name == "nt":
            self.skipTest("windows is too flaky for this test")

    def build_under(self, root, name, latency: float = 0) -> None:
        os.mkdir(os.path.join(root, name))
        if latency > 0:
            time.sleep(latency)
        self.touch(os.path.join(root, name, "a"))

    def test_atomicMove(self) -> None:
        root = self.mkdtemp()

        dir_of_interest = os.path.join(root, "dir")
        alt_dir = os.path.join(root, "alt")
        dead_dir = os.path.join(root, "bye")

        self.build_under(root, "dir")
        self.watchmanCommand("watch", root)
        self.assertFileList(root, ["dir", "dir/a"])

        # build out a replacement dir
        self.build_under(root, "alt")

        os.rename(dir_of_interest, dead_dir)
        os.rename(alt_dir, dir_of_interest)

        self.assertFileList(root, ["dir", "dir/a", "bye", "bye/a"])

    def test_NonAtomicMove(self) -> None:
        root = self.mkdtemp()

        dir_of_interest = os.path.join(root, "dir")

        self.build_under(root, "dir")
        self.watchmanCommand("watch", root)
        self.assertFileList(root, ["dir", "dir/a"])

        shutil.rmtree(dir_of_interest)
        self.build_under(root, "dir", latency=1)

        self.assertFileList(root, ["dir", "dir/a"])
