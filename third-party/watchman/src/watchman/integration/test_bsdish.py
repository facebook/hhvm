# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import os

from watchman.integration.lib import WatchmanTestCase


@WatchmanTestCase.expand_matrix
class TestBSDish(WatchmanTestCase.WatchmanTestCase):
    def test_bsdish_toplevel(self) -> None:
        root = self.mkdtemp()
        os.mkdir(os.path.join(root, "lower"))
        self.touchRelative(root, "lower", "file")
        self.touchRelative(root, "top")

        watch = self.watchmanCommand("watch", root)

        self.assertFileList(root, ["lower", "lower/file", "top"])

        find = self.watchmanCommand("find", root)
        clock = find["clock"]

        since = self.watchmanCommand("since", root, clock)
        clock = since["clock"]

        since = self.watchmanCommand(
            "query", root, {"expression": ["allof", ["since", clock], ["type", "f"]]}
        )
        self.assertFileListsEqual([], since["files"])
        clock = since["clock"]

        os.unlink(os.path.join(root, "top"))
        self.assertFileList(root, ["lower", "lower/file"])

        now = self.watchmanCommand("since", root, clock)
        expected = ["top"]
        if watch["watcher"] == "kqueue+fsevents":
            # For the split watch, a cookie is being written to each top level
            # directory, and thus the "lower" directory will be reported as
            # having been changed.
            expected.append("lower")
        self.assertEqual(len(expected), len(now["files"]))
        self.assertFileListsEqual(
            expected, list(map(lambda x: x["name"], now["files"]))
        )
        for f in now["files"]:
            if f["name"] == "top":
                self.assertFalse(f["exists"])
