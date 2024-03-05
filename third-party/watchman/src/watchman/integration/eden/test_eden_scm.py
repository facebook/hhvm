# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe

from watchman.integration.lib import WatchmanEdenTestCase
from watchman.integration.lib.WatchmanSCMTestCase import HgMixin


def populate(repo) -> None:
    # We ignore ".hg" here just so some of the tests that list files don't have to
    # explicitly filter out the contents of this directory.  However, in most situations
    # the .hg directory normally should not be ignored.
    repo.write_file(".watchmanconfig", '{"ignore_dirs":[".hg"]}')
    repo.write_file("hello", "hola\n")
    repo.commit("initial commit.")


class TestEdenScm(WatchmanEdenTestCase.WatchmanEdenTestCase, HgMixin):
    def test_eden_cachedScm(self) -> None:
        root = self.makeEdenMount(populate)
        res = self.watchmanCommand("watch", root)
        self.assertEqual("eden", res["watcher"])

        self.hg(["book", "initial"], root)

        def run_scm_query():
            res = self.watchmanCommand(
                "query",
                root,
                {
                    "fields": ["name"],
                    "since": {"scm": {"mergebase-with": "initial"}},
                },
            )
            self.assertNotEqual(res["clock"]["scm"]["mergebase"], "")
            self.assertEqual(res["clock"]["scm"]["mergebase-with"], "initial")
            return res

        self.touchRelative(root, "foo")
        res = run_scm_query()
        self.assertFileListsEqual(res["files"], ["foo"])

        self.touchRelative(root, "bar")
        res = run_scm_query()
        self.assertFileListsEqual(res["files"], ["foo", "bar"])

        self.touchRelative(root, "baz")
        res = run_scm_query()
        self.assertFileListsEqual(res["files"], ["foo", "bar", "baz"])
