# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import os

from watchman.integration.lib import WatchmanTestCase


@WatchmanTestCase.expand_matrix
class TestDirName(WatchmanTestCase.WatchmanTestCase):
    def test_dirname(self) -> None:
        root = self.mkdtemp()
        for i in range(0, 5):
            istr = str(i)
            os.makedirs(os.path.join(root, istr, istr, istr, istr, istr))
            self.touchRelative(root, "a")
            self.touchRelative(root, istr, "a")
            self.touchRelative(root, "%sa" % istr)
            self.touchRelative(root, istr, istr, "a")
            self.touchRelative(root, istr, istr, istr, "a")
            self.touchRelative(root, istr, istr, istr, istr, "a")
            self.touchRelative(root, istr, istr, istr, istr, istr, "a")

        self.watchmanCommand("watch", root)

        tests = [
            [
                "",
                None,
                [
                    "0/0/0/0/0/a",
                    "0/0/0/0/a",
                    "0/0/0/a",
                    "0/0/a",
                    "0/a",
                    "1/1/1/1/1/a",
                    "1/1/1/1/a",
                    "1/1/1/a",
                    "1/1/a",
                    "1/a",
                    "2/2/2/2/2/a",
                    "2/2/2/2/a",
                    "2/2/2/a",
                    "2/2/a",
                    "2/a",
                    "3/3/3/3/3/a",
                    "3/3/3/3/a",
                    "3/3/3/a",
                    "3/3/a",
                    "3/a",
                    "4/4/4/4/4/a",
                    "4/4/4/4/a",
                    "4/4/4/a",
                    "4/4/a",
                    "4/a",
                    "a",
                ],
            ],
            [
                "",
                4,
                [
                    "0/0/0/0/0/a",
                    "1/1/1/1/1/a",
                    "2/2/2/2/2/a",
                    "3/3/3/3/3/a",
                    "4/4/4/4/4/a",
                ],
            ],
            [
                "",
                3,
                [
                    "0/0/0/0/0/a",
                    "0/0/0/0/a",
                    "1/1/1/1/1/a",
                    "1/1/1/1/a",
                    "2/2/2/2/2/a",
                    "2/2/2/2/a",
                    "3/3/3/3/3/a",
                    "3/3/3/3/a",
                    "4/4/4/4/4/a",
                    "4/4/4/4/a",
                ],
            ],
            ["0", None, ["0/0/0/0/0/a", "0/0/0/0/a", "0/0/0/a", "0/0/a", "0/a"]],
            ["1", None, ["1/1/1/1/1/a", "1/1/1/1/a", "1/1/1/a", "1/1/a", "1/a"]],
            ["1", 0, ["1/1/1/1/1/a", "1/1/1/1/a", "1/1/1/a", "1/1/a"]],
            ["1", 1, ["1/1/1/1/1/a", "1/1/1/1/a", "1/1/1/a"]],
            ["1", 2, ["1/1/1/1/1/a", "1/1/1/1/a"]],
            ["1", 3, ["1/1/1/1/1/a"]],
            ["1", 4, []],
        ]

        for dirname, depth, expect in tests:
            if depth is None:
                # equivalent to `depth ge 0`
                term = ["dirname", dirname]
            else:
                term = ["dirname", dirname, ["depth", "gt", depth]]

            label = repr([dirname, depth, expect, term])

            results = self.watchmanCommand(
                "query",
                root,
                {"expression": ["allof", term, ["name", "a"]], "fields": ["name"]},
            )

            self.assertFileListsEqual(results["files"], expect, label)
