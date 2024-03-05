# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import os

from watchman.integration.lib import WatchmanTestCase


@WatchmanTestCase.expand_matrix
class TestCaseSensitive(WatchmanTestCase.WatchmanTestCase):
    def test_changeCase(self) -> None:
        root = self.mkdtemp()
        os.mkdir(os.path.join(root, "foo"))
        self.watchmanCommand("watch", root)
        self.assertFileList(root, ["foo"])

        os.rename(os.path.join(root, "foo"), os.path.join(root, "FOO"))
        self.touchRelative(root, "FOO", "bar")
        self.assertFileList(root, ["FOO", "FOO/bar"])

        os.rename(os.path.join(root, "FOO", "bar"), os.path.join(root, "FOO", "BAR"))
        self.assertFileList(root, ["FOO", "FOO/BAR"])

        os.rename(os.path.join(root, "FOO"), os.path.join(root, "foo"))
        self.assertFileList(root, ["foo", "foo/BAR"])

        os.mkdir(os.path.join(root, "foo", "baz"))
        self.touchRelative(root, "foo", "baz", "file")
        self.assertFileList(root, ["foo", "foo/BAR", "foo/baz", "foo/baz/file"])

        os.rename(os.path.join(root, "foo"), os.path.join(root, "Foo"))

        self.assertFileList(root, ["Foo", "Foo/BAR", "Foo/baz", "Foo/baz/file"])
