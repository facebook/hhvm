# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import os

from watchman.integration.lib import WatchmanTestCase


@WatchmanTestCase.expand_matrix
class TestPathGenerator(WatchmanTestCase.WatchmanTestCase):
    def test_path_generator_dot(self) -> None:
        root = self.mkdtemp()

        self.watchmanCommand("watch", root)
        self.assertFileListsEqual(
            self.watchmanCommand("query", root, {"path": ["."]})["files"], []
        )

        self.assertFileListsEqual(
            self.watchmanCommand("query", root, {"relative_root": ".", "path": ["."]})[
                "files"
            ],
            [],
        )

    def test_path_generator_case(self) -> None:
        root = self.mkdtemp()

        os.mkdir(os.path.join(root, "foo"))
        self.touchRelative(root, "foo", "bar")
        self.watchmanCommand("watch", root)

        self.assertFileListsEqual(
            self.watchmanCommand("query", root, {"fields": ["name"], "path": ["foo"]})[
                "files"
            ],
            ["foo/bar"],
        )

        if self.isCaseInsensitive():
            os.rename(os.path.join(root, "foo"), os.path.join(root, "Foo"))

            self.assertFileListsEqual(
                self.watchmanCommand(
                    "query", root, {"fields": ["name"], "path": ["foo"]}  # not Foo!
                )["files"],
                [],
                message="Case insensitive matching not implemented \
                        for path generator",
            )

    def test_path_generator_relative_root(self) -> None:
        root = self.mkdtemp()

        os.mkdir(os.path.join(root, "foo"))
        self.touchRelative(root, "foo", "bar")
        self.watchmanCommand("watch", root)

        self.assertFileListsEqual(
            self.watchmanCommand(
                "query",
                root,
                {"fields": ["name"], "relative_root": "foo", "path": ["bar"]},
            )["files"],
            ["bar"],
        )

        self.assertFileListsEqual(
            self.watchmanCommand(
                "query",
                root,
                {
                    "fields": ["name"],
                    "relative_root": "foo",
                    "path": [{"path": "bar", "depth": -1}],
                },
            )["files"],
            ["bar"],
        )

        if self.isCaseInsensitive():
            os.rename(os.path.join(root, "foo"), os.path.join(root, "Foo"))

            self.assertFileListsEqual(
                self.watchmanCommand(
                    "query", root, {"fields": ["name"], "path": ["foo"]}  # not Foo!
                )["files"],
                [],
                message="Case insensitive matching not implemented \
                        for path relative_root",
            )

    def test_path_generator_empty(self) -> None:
        """Specifying no input paths should return no results."""
        root = self.mkdtemp()

        os.mkdir(os.path.join(root, "mydir"))
        self.touchRelative(root, "myfile")
        self.watchmanCommand("watch", root)

        self.assertFileListsEqual(
            self.watchmanCommand("query", root, {"fields": ["name"], "path": []})[
                "files"
            ],
            [],
        )
