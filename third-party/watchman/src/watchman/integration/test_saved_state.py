# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import pywatchman
from watchman.integration.lib import WatchmanSCMTestCase, WatchmanTestCase


@WatchmanTestCase.expand_matrix
class TestSavedState(WatchmanSCMTestCase.WatchmanSCMTestCase):
    def setUp(self) -> None:
        self.skipIfNoFSMonitor()
        self.root = self.mkdtemp()
        # This test does not require much so just create a super simple repo
        self.hg(["init"], cwd=self.root)
        self.touchRelative(self.root, "foo")
        self.hg(["book", "initial"], cwd=self.root)
        self.hg(["addremove"], cwd=self.root)
        self.hg(["commit", "-m", "initial"], cwd=self.root)
        self.touchRelative(self.root, "bar")
        self.touchRelative(self.root, "car")
        self.hg(["addremove"], cwd=self.root)
        self.hg(["commit", "-m", "add bar and car"], cwd=self.root)
        self.hg(["book", "main"], cwd=self.root)
        self.watchmanCommand("watch", self.root)

    def get_skeleton_query(self):
        return {
            "expression": [
                "not",
                ["anyof", ["name", ".hg"], ["match", "hg-check*"], ["dirname", ".hg"]],
            ],
            "fields": ["name"],
            "since": {"scm": {"mergebase-with": "main"}},
        }

    def test_unsupportedStorageType(self) -> None:
        # If the storage type is not supported, watchman should throw
        test_query = self.get_skeleton_query()
        test_query["since"]["scm"]["saved-state"] = {"storage": "foo", "config": {}}
        with self.assertRaises(pywatchman.WatchmanError) as ctx:
            self.watchmanCommand("query", self.root, test_query)
        self.assertIn("invalid storage type 'foo'", str(ctx.exception))
