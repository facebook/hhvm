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
class TestSinceTerm(WatchmanTestCase.WatchmanTestCase):
    def test_since_term(self) -> None:
        root = self.mkdtemp()

        self.touchRelative(root, "foo.c")
        os.mkdir(os.path.join(root, "subdir"))
        self.touchRelative(root, "subdir", "bar.txt")

        watch = self.watchmanCommand("watch", root)

        res = self.watchmanCommand("find", root, "foo.c")
        first_clock = res["clock"]
        base_mtime = res["files"][0]["mtime"]

        # Since is GT not GTE
        res = self.watchmanCommand(
            "query",
            root,
            {"expression": ["since", base_mtime, "mtime"], "fields": ["name"]},
        )
        self.assertFileListsEqual(res["files"], ["foo.c", "subdir", "subdir/bar.txt"])

        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": [
                    "allof",
                    ["since", base_mtime - 1, "mtime"],
                    ["name", "foo.c"],
                ],
                "fields": ["name"],
            },
        )
        self.assertFileListsEqual(res["files"], ["foo.c"])

        if self.isCaseInsensitive():
            res = self.watchmanCommand(
                "query",
                root,
                {
                    "expression": [
                        "allof",
                        ["since", base_mtime - 1, "mtime"],
                        ["name", "FOO.c"],
                    ],
                    "fields": ["name"],
                },
            )
            self.assertFileListsEqual(res["files"], ["foo.c"])

        # Try with a clock
        res = self.watchmanCommand(
            "query", root, {"expression": ["since", first_clock], "fields": ["name"]}
        )
        expected = []
        if watch["watcher"] == "kqueue+fsevents":
            # A cookie is written to subdir in the split watcher, thus it is
            # expected to have it in the files returned by the query.
            expected.append("subdir")
        self.assertFileListsEqual(res["files"], expected)

        future = base_mtime + 15
        self.touch(os.path.join(root, "foo.c"), (future, future))

        # Try again with a clock
        res = self.watchmanCommand(
            "query", root, {"expression": ["since", first_clock], "fields": ["name"]}
        )
        self.assertFileListsEqual(res["files"], ["foo.c"] + expected)

        # And check that we're still later than a later but not current mtime
        res = self.watchmanCommand(
            "query",
            root,
            {"expression": ["since", base_mtime + 5, "mtime"], "fields": ["name"]},
        )
        self.assertFileListsEqual(res["files"], ["foo.c"])

        # If using a timestamp against the oclock, ensure that we're comparing
        # in the correct order.  We need to force a 2 second delay so that the
        # timestamp moves forward by at least 1 increment for this test to
        # work correctly
        time.sleep(2)

        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": ["allof", ["since", int(time.time())], ["name", "foo.c"]],
                "fields": ["name"],
            },
        )
        self.assertFileListsEqual(res["files"], [])

        # Try with a fresh clock instance; we must only return files that exist.
        self.removeRelative(root, "subdir", "bar.txt")
        self.assertFileList(root, files=["foo.c", "subdir"], cursor="c:0:0")
