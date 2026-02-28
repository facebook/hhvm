# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import json
import os

import pywatchman
from watchman.integration.lib import WatchmanTestCase


@WatchmanTestCase.expand_matrix
class TestIgnore(WatchmanTestCase.WatchmanTestCase):
    def test_ignore_git(self) -> None:
        root = self.mkdtemp()
        os.mkdir(os.path.join(root, ".git"))
        os.mkdir(os.path.join(root, ".git", "objects"))
        os.mkdir(os.path.join(root, ".git", "objects", "pack"))
        self.touchRelative(root, "foo")

        self.watchmanCommand("watch", root)

        # prove that we don't see pack in .git as we crawl
        self.assertFileList(root, files=[".git", ".git/objects", "foo"])

        # and prove that we aren't watching deeply under .git
        self.touchRelative(root, ".git", "objects", "dontlookatme")
        self.assertFileList(root, files=[".git", ".git/objects", "foo"])

    def test_invalid_ignore(self) -> None:
        root = self.mkdtemp()
        bad = [{"ignore_vcs": "lemon"}, {"ignore_vcs": ["foo", 123]}]
        for cfg in bad:
            with open(os.path.join(root, ".watchmanconfig"), "w") as f:
                json.dump(cfg, f)

            with self.assertRaises(pywatchman.WatchmanError) as ctx:
                self.watchmanCommand("watch", root)
            self.assertIn("ignore_vcs must be an array of strings", str(ctx.exception))

    def test_ignore_overlap_vcs_ignore(self) -> None:
        """Validate that we still have working cookies even though we were
        told to ignore .hg"""
        root = self.mkdtemp()
        with open(os.path.join(root, ".watchmanconfig"), "w") as f:
            json.dump({"ignore_dirs": [".hg"]}, f)
        os.mkdir(os.path.join(root, ".hg"))

        self.watchmanCommand("watch", root)
        self.assertFileList(root, files=[".watchmanconfig"])
        self.touchRelative(root, "foo")
        self.assertFileList(root, files=[".watchmanconfig", "foo"])

    def test_ignore_generic(self) -> None:
        root = self.mkdtemp()
        with open(os.path.join(root, ".watchmanconfig"), "w") as f:
            json.dump({"ignore_dirs": ["build"]}, f)
        os.makedirs(os.path.join(root, "build", "lower"))
        os.makedirs(os.path.join(root, "builda"))
        self.touchRelative(root, "foo")
        self.touchRelative(root, "build", "bar")
        self.touchRelative(root, "buildfile")
        self.touchRelative(root, "build", "lower", "baz")
        self.touchRelative(root, "builda", "hello")

        self.watchmanCommand("watch", root)
        self.assertFileList(
            root,
            files=[".watchmanconfig", "builda", "builda/hello", "buildfile", "foo"],
        )

        self.touchRelative(root, "build", "lower", "dontlookatme")
        self.touchRelative(root, "build", "orme")
        self.touchRelative(root, "buil")

        self.assertFileList(
            root,
            files=[
                ".watchmanconfig",
                "buil",
                "builda",
                "builda/hello",
                "buildfile",
                "foo",
            ],
        )
