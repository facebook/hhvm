# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import os
import os.path
import shutil

import pywatchman
from watchman.integration.lib import WatchmanTestCase


@WatchmanTestCase.expand_matrix
class TestGlob(WatchmanTestCase.WatchmanTestCase):
    def test_glob(self) -> None:
        root = self.mkdtemp()
        self.touchRelative(root, "a.c")
        self.touchRelative(root, "b.c")
        self.touchRelative(root, ".a.c")

        inc_dir = os.path.join(root, "includes")
        os.mkdir(inc_dir)
        self.touchRelative(inc_dir, "a.h")
        self.touchRelative(inc_dir, "b.h")

        second_inc_dir = os.path.join(inc_dir, "second")
        os.mkdir(second_inc_dir)
        self.touchRelative(second_inc_dir, "foo.h")
        self.touchRelative(second_inc_dir, "bar.h")

        self.watchmanCommand("watch", root)

        res = self.watchmanCommand("query", root, {"glob": ["*.h"], "fields": ["name"]})
        self.assertEqual(res["files"], [])

        res = self.watchmanCommand(
            "query",
            root,
            {"glob": ["*.h"], "relative_root": "includes", "fields": ["name"]},
        )
        self.assertFileListsEqual(["a.h", "b.h"], res["files"])

        res = self.watchmanCommand(
            "query", root, {"glob": ["**/*.h"], "fields": ["name"]}
        )
        self.assertFileListsEqual(
            [
                "includes/a.h",
                "includes/b.h",
                "includes/second/bar.h",
                "includes/second/foo.h",
            ],
            res["files"],
        )

        res = self.watchmanCommand(
            "query",
            root,
            {
                "glob": ["**/*.h"],
                "relative_root": "includes/second",
                "fields": ["name"],
            },
        )
        self.assertFileListsEqual(["bar.h", "foo.h"], res["files"])

        # check that a windows style path separator is normalized when
        # present in relative_root
        res = self.watchmanCommand(
            "query",
            root,
            {
                "glob": ["**/*.h"],
                "relative_root": "includes\\second",
                "fields": ["name"],
            },
        )
        self.assertFileListsEqual(["bar.h", "foo.h"], res["files"])

        res = self.watchmanCommand(
            "query",
            root,
            {"glob": ["**/*.h"], "relative_root": "includes", "fields": ["name"]},
        )
        self.assertFileListsEqual(
            ["a.h", "b.h", "second/bar.h", "second/foo.h"], res["files"]
        )

        res = self.watchmanCommand("query", root, {"glob": ["*.c"], "fields": ["name"]})
        self.assertFileListsEqual(res["files"], ["a.c", "b.c"])

        res = self.watchmanCommand(
            "query",
            root,
            {"glob": ["*.c"], "glob_includedotfiles": True, "fields": ["name"]},
        )
        self.assertFileListsEqual(res["files"], [".a.c", "a.c", "b.c"])

        res = self.watchmanCommand(
            "query", root, {"glob": ["**/*.h", "**/**/*.h"], "fields": ["name"]}
        )
        self.assertFileListsEqual(
            [
                "includes/a.h",
                "includes/b.h",
                "includes/second/bar.h",
                "includes/second/foo.h",
            ],
            res["files"],
        )

        # check that dedup is happening
        res = self.watchmanCommand(
            "query",
            root,
            {"glob": ["**/*.h", "**/**/*.h", "includes/*.h"], "fields": ["name"]},
        )
        self.assertFileListsEqual(
            [
                "includes/a.h",
                "includes/b.h",
                "includes/second/bar.h",
                "includes/second/foo.h",
            ],
            res["files"],
        )

        shutil.rmtree(second_inc_dir)

        res = self.watchmanCommand(
            "query", root, {"glob": ["**/*.h", "**/**/*.h"], "fields": ["name"]}
        )
        self.assertFileListsEqual(["includes/a.h", "includes/b.h"], res["files"])

        res = self.watchmanCommand(
            "query", root, {"glob": ["*/*.h"], "fields": ["name"]}
        )
        self.assertFileListsEqual(["includes/a.h", "includes/b.h"], res["files"])

        os.unlink(os.path.join(inc_dir, "a.h"))

        res = self.watchmanCommand(
            "query", root, {"glob": ["*/*.h"], "fields": ["name"]}
        )
        self.assertFileListsEqual(["includes/b.h"], res["files"])

        with self.assertRaises(pywatchman.WatchmanError) as ctx:
            self.watchmanCommand(
                "query", root, {"glob": ["*/*.h"], "relative_root": "bogus"}
            )
        self.assertIn("check your relative_root", str(ctx.exception))

        with self.assertRaises(pywatchman.WatchmanError) as ctx:
            self.watchmanCommand("query", root, {"glob": [12345]})
        self.assertIn("expected json string object", str(ctx.exception))

    def test_glob_generator_empty(self) -> None:
        """Specifying no input patterns should return no results."""
        root = self.mkdtemp()

        os.mkdir(os.path.join(root, "mydir"))
        self.touchRelative(root, "myfile")
        self.watchmanCommand("watch", root)

        res = self.watchmanCommand("query", root, {"fields": ["name"], "glob": []})
        self.assertFileListsEqual(res["files"], [])

    def test_glob_generator_absolute(self) -> None:
        """Make it easier to understand buck errors resulting from bad globs"""
        root = self.mkdtemp()

        os.mkdir(os.path.join(root, "mydir"))
        self.watchmanCommand("watch", root)

        with self.assertRaises(pywatchman.WatchmanError) as ctx:
            self.watchmanCommand("query", root, {"glob": ["//fbandroid/*.cpp"]})
        self.assertIn(
            "QueryParseError: failed to parse query: "
            "glob `//fbandroid/*.cpp` is an absolute path.  "
            "All globs must be relative paths",
            str(ctx.exception),
        )

    def test_case_sensitive(self) -> None:
        root = self.mkdtemp()

        os.mkdir(os.path.join(root, "Hello"))
        self.touchRelative(root, "hello.c")

        self.watchmanCommand("watch", root)

        expect_sensitive = ["Hello"]
        expect_insensitive = ["Hello", "hello.c"]

        res = self.watchmanCommand(
            "query",
            root,
            {"fields": ["name"], "glob": ["Hello*"], "case_sensitive": True},
        )
        self.assertFileListsEqual(res["files"], expect_sensitive)

        res = self.watchmanCommand(
            "query",
            root,
            {"fields": ["name"], "glob": ["Hello*"], "case_sensitive": False},
        )
        self.assertFileListsEqual(res["files"], expect_insensitive)

        res = self.watchmanCommand(
            "query", root, {"fields": ["name"], "glob": ["Hello*"]}
        )
        self.assertFileListsEqual(
            res["files"],
            expect_insensitive if self.isCaseInsensitive() else expect_sensitive,
        )
