# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import os

from watchman.integration.lib import WatchmanTestCase


@WatchmanTestCase.expand_matrix
class TestEmpty(WatchmanTestCase.WatchmanTestCase):
    def test_empty(self) -> None:
        root = self.mkdtemp()
        self.touchRelative(root, "empty")
        with open(os.path.join(root, "notempty"), "w") as f:
            f.write("foo")

        self.watchmanCommand("watch", root)
        results = self.watchmanCommand(
            "query", root, {"expression": "empty", "fields": ["name"]}
        )

        self.assertEqual(["empty"], results["files"])

        results = self.watchmanCommand(
            "query", root, {"expression": "exists", "fields": ["name"]}
        )

        self.assertFileListsEqual(["empty", "notempty"], results["files"])

        clock = results["clock"]
        os.unlink(os.path.join(root, "empty"))

        self.assertFileList(root, files=["notempty"])

        results = self.watchmanCommand(
            "query", root, {"expression": "exists", "fields": ["name"]}
        )

        self.assertFileListsEqual(["notempty"], results["files"])

        # "files that don't exist" without a since term is absurd, so pass that in
        results = self.watchmanCommand(
            "query",
            root,
            {"since": clock, "expression": ["not", "exists"], "fields": ["name"]},
        )

        self.assertFileListsEqual(["empty"], results["files"])
