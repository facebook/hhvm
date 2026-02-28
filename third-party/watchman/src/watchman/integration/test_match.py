# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import os
import os.path

from watchman.integration.lib import WatchmanTestCase


@WatchmanTestCase.expand_matrix
class TestMatch(WatchmanTestCase.WatchmanTestCase):
    def test_match(self) -> None:
        root = self.mkdtemp()
        self.touchRelative(root, "foo.c")
        self.touchRelative(root, "bar.txt")
        os.mkdir(os.path.join(root, "foo"))
        self.touchRelative(root, "foo", ".bar.c")
        self.touchRelative(root, "foo", "baz.c")

        self.watchmanCommand("watch", root)

        self.assertFileList(
            root, ["bar.txt", "foo.c", "foo", "foo/.bar.c", "foo/baz.c"]
        )

        res = self.watchmanCommand(
            "query", root, {"expression": ["match", "*.c"], "fields": ["name"]}
        )
        self.assertFileListsEqual(res["files"], ["foo.c", "foo/baz.c"])

        res = self.watchmanCommand(
            "query",
            root,
            {"expression": ["match", "*.c", "wholename"], "fields": ["name"]},
        )
        self.assertFileListsEqual(res["files"], ["foo.c"])

        res = self.watchmanCommand(
            "query",
            root,
            {"expression": ["match", "foo/*.c", "wholename"], "fields": ["name"]},
        )
        self.assertFileListsEqual(res["files"], ["foo/baz.c"])

        res = self.watchmanCommand(
            "query",
            root,
            {"expression": ["match", "foo/*.c", "wholename"], "fields": ["name"]},
        )
        self.assertFileListsEqual(res["files"], ["foo/baz.c"])

        res = self.watchmanCommand(
            "query",
            root,
            {"expression": ["match", "**/*.c", "wholename"], "fields": ["name"]},
        )
        self.assertFileListsEqual(res["files"], ["foo.c", "foo/baz.c"])

        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": [
                    "match",
                    "**/*.c",
                    "wholename",
                    {"includedotfiles": True},
                ],
                "fields": ["name"],
            },
        )
        self.assertFileListsEqual(res["files"], ["foo.c", "foo/.bar.c", "foo/baz.c"])

        res = self.watchmanCommand(
            "query",
            root,
            {"expression": ["match", "foo/**/*.c", "wholename"], "fields": ["name"]},
        )
        self.assertFileListsEqual(res["files"], ["foo/baz.c"])

        res = self.watchmanCommand(
            "query",
            root,
            {"expression": ["match", "FOO/*.c", "wholename"], "fields": ["name"]},
        )
        if self.isCaseInsensitive():
            self.assertFileListsEqual(res["files"], ["foo/baz.c"])
        else:
            self.assertFileListsEqual(res["files"], [])

        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": ["match", "FOO/*.c", "wholename"],
                "case_sensitive": True,
                "fields": ["name"],
            },
        )
        self.assertFileListsEqual(res["files"], [])

        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": ["match", "FOO/*.c", "wholename"],
                "case_sensitive": False,
                "fields": ["name"],
            },
        )
        self.assertFileListsEqual(res["files"], ["foo/baz.c"])
