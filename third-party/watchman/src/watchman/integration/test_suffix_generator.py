# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import os

import pywatchman
from watchman.integration.lib import WatchmanTestCase


@WatchmanTestCase.expand_matrix
class TestSuffixGenerator(WatchmanTestCase.WatchmanTestCase):
    def test_suffix_generator(self) -> None:
        root = self.mkdtemp()

        # Suffix queries are defined as being case insensitive.
        # Use an uppercase suffix to verify that our lowercase
        # suffixes in the pattern are matching correctly.
        self.touchRelative(root, "foo.C")
        os.mkdir(os.path.join(root, "subdir"))
        self.touchRelative(root, "subdir", "bar.txt")

        self.watchmanCommand("watch", root)

        self.assertFileListsEqual(
            self.watchmanCommand("query", root, {"suffix": "c", "fields": ["name"]})[
                "files"
            ],
            ["foo.C"],
        )

        self.assertFileListsEqual(
            self.watchmanCommand(
                "query", root, {"suffix": ["c", "txt"], "fields": ["name"]}
            )["files"],
            ["foo.C", "subdir/bar.txt"],
        )

        self.assertFileListsEqual(
            self.watchmanCommand(
                "query",
                root,
                {"suffix": ["c", "txt"], "relative_root": "subdir", "fields": ["name"]},
            )["files"],
            ["bar.txt"],
        )

        with self.assertRaises(pywatchman.WatchmanError) as ctx:
            self.watchmanCommand("query", root, {"suffix": {"a": "b"}})

        self.assertRegex(
            str(ctx.exception), "'suffix' must be a string or an array of strings"
        )

    def test_suffix_generator_empty(self) -> None:
        """Specifying no input suffixes should return no results."""
        root = self.mkdtemp()

        os.mkdir(os.path.join(root, "mydir"))
        os.mkdir(os.path.join(root, "mydir.dir"))
        self.touchRelative(root, "myfile")
        self.touchRelative(root, "myfile.txt")
        self.watchmanCommand("watch", root)

        self.assertFileListsEqual(
            self.watchmanCommand("query", root, {"fields": ["name"], "suffix": []})[
                "files"
            ],
            [],
        )
