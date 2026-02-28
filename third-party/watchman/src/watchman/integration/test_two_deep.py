# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import os
import time

from watchman.integration.lib import WatchmanTestCase


@WatchmanTestCase.expand_matrix
class TestTwoDeep(WatchmanTestCase.WatchmanTestCase):
    def test_two_deep(self) -> None:
        root = self.mkdtemp()
        self.watchmanCommand("watch", root)

        os.makedirs(os.path.join(root, "foo", "bar"))

        # Guarantee that 111's mtime is greater than its parent dirs
        time.sleep(1)

        with open(os.path.join(root, "foo", "bar", "111"), "w") as f:
            f.write("111")

        self.assertFileList(root, files=["foo", "foo/bar", "foo/bar/111"])

        res_111 = self.watchmanCommand("find", root, "foo/bar/111")["files"][0]
        st_111 = os.lstat(os.path.join(root, "foo", "bar", "111"))

        res_bar = self.watchmanCommand("find", root, "foo/bar")["files"][0]
        st_bar = os.lstat(os.path.join(root, "foo", "bar"))

        self.assertEqual(res_111["mtime"], int(st_111.st_mtime))
        self.assertEqual(res_bar["mtime"], int(st_bar.st_mtime))
