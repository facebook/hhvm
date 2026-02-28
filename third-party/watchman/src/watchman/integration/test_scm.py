# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import os
import unittest

import pywatchman
from watchman.integration.lib import WatchmanSCMTestCase, WatchmanTestCase


def is_ubuntu() -> bool:
    try:
        with open("/etc/lsb-release") as f:
            if "Ubuntu" in f.read():
                return True
    except Exception:
        pass
    return False


@WatchmanTestCase.expand_matrix
class TestScm(WatchmanSCMTestCase.WatchmanSCMTestCase):
    def test_not_supported(self) -> None:
        root = self.mkdtemp()
        self.watchmanCommand("watch", root)

        with self.assertRaises(pywatchman.WatchmanError) as ctx:
            self.watchmanCommand(
                "query",
                root,
                {
                    "expression": ["allof", ["type", "f"], ["match", "*.sh"]],
                    "fields": ["name"],
                    "since": {
                        "scm": {"mergebase-with": "remote/master"},
                        "clock": "c:0:0",
                    },
                },
            )
        self.assertIn("root does not support SCM-aware queries", str(ctx.exception))

    @unittest.skipIf(is_ubuntu(), "Test is flaky. See Facebook task T36574087.")
    def test_scmHg(self) -> None:
        self.skipIfNoFSMonitor()

        root = self.mkdtemp()
        """ Set up a repo with a DAG like this:
@  changeset:   4:6c38b3c78a62
|  bookmark:    feature2
|  tag:         tip
|  summary:     add m2
|
o  changeset:   3:88fea8704cd2
|  bookmark:    main
|  parent:      1:6b3ecb11785e
|  summary:     add m1
|
| o  changeset:   5:7bc34583612
|/   bookmark:    feature3
|    summary:     remove car
|
| o  changeset:   2:2db357583971
|/   bookmark:    feature1
|    summary:     add f1
|
o  changeset:   0:b08db10380dd
   bookmark:    initial
   summary:     initial
        """

        self.hg(["init"], cwd=root)
        self.touchRelative(root, "foo")
        self.hg(["book", "initial"], cwd=root)
        self.hg(["addremove"], cwd=root)
        self.hg(["commit", "-m", "initial"], cwd=root)
        self.hg(["book", "main"], cwd=root)
        self.touchRelative(root, "bar")
        self.touchRelative(root, "car")
        self.hg(["addremove"], cwd=root)
        self.hg(["commit", "-m", "add bar and car"], cwd=root)
        self.hg(["book", "feature1"], cwd=root)
        os.makedirs(os.path.join(root, "a", "b", "c"))
        self.touchRelative(root, "a", "b", "c", "f1")
        self.hg(["addremove"], cwd=root)
        self.hg(["commit", "-m", "add f1"], cwd=root)
        self.hg(["co", "main"], cwd=root)
        self.hg(["book", "feature3"], cwd=root)
        self.hg(["rm", "car"], cwd=root)
        self.hg(["commit", "-m", "remove car"], cwd=root)
        self.hg(["co", "main"], cwd=root)
        self.touchRelative(root, "m1")
        self.hg(["addremove"], cwd=root)
        self.hg(["commit", "-m", "add m1"], cwd=root)
        self.hg(["book", "feature2"], cwd=root)
        self.touchRelative(root, "m2")
        self.hg(["addremove"], cwd=root)
        self.hg(["commit", "-m", "add m2"], cwd=root)

        watch = self.watchmanCommand("watch", root)

        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": [
                    "not",
                    [
                        "anyof",
                        ["name", ".hg"],
                        ["match", "hg-check*"],
                        ["dirname", ".hg"],
                    ],
                ],
                "fields": ["name"],
            },
        )
        self.assertFileListsEqual(res["files"], ["foo", "bar", "car", "m1", "m2"])

        # Verify behavior with badly formed queries
        with self.assertRaises(pywatchman.WatchmanError) as ctx:
            self.watchmanCommand(
                "query",
                root,
                {
                    "expression": [
                        "not",
                        [
                            "anyof",
                            ["name", ".hg"],
                            ["match", "hg-check*"],
                            ["dirname", ".hg"],
                        ],
                    ],
                    "since": {"scm": {}},
                },
            )
        self.assertIn(
            "key 'mergebase-with' is not present in this json object",
            str(ctx.exception),
        )

        # When the client doesn't know the merge base, we should give
        # them the current status and merge base
        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": [
                    "not",
                    [
                        "anyof",
                        ["name", ".hg"],
                        ["match", "hg-check*"],
                        ["dirname", ".hg"],
                    ],
                ],
                "fields": ["name"],
                "since": {"scm": {"mergebase-with": "main"}},
            },
        )

        self.assertNotEqual(res["clock"]["scm"]["mergebase"], "")
        self.assertEqual(res["clock"]["scm"]["mergebase-with"], "main")
        # The only file changed between main and feature2 is m2
        self.assertFileListsEqual(res["files"], ["m2"])

        # Let's also set up a subscription for the same query
        sub = self.watchmanCommand(
            "subscribe",
            root,
            "scmsub",
            {
                "expression": [
                    "not",
                    [
                        "anyof",
                        ["name", ".hg"],
                        ["match", "hg-check*"],
                        ["dirname", ".hg"],
                    ],
                ],
                "fields": ["name"],
                "since": {"scm": {"mergebase-with": "main"}},
            },
        )

        self.waitForStatesToVacate(root)
        self.watchmanCommand("flush-subscriptions", root, {"sync_timeout": 1000})
        dat = self.getSubFatClocksOnly("scmsub", root=root)

        # compare with the query results that we got
        self.assertEqual(sub["clock"]["scm"], res["clock"]["scm"])
        self.assertFileListsEqual(res["files"], self.getConsolidatedFileList(dat))

        mergeBase = res["clock"]["scm"]["mergebase"]

        # Ensure that we can see a file that isn't tracked show up
        # as a delta in the what we consider to be the common case.
        # we're threading the merge-base result from the prior query
        # through, so this should just end up looking like a normal
        # since query.
        self.touchRelative(root, "w00t")
        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": [
                    "not",
                    [
                        "anyof",
                        ["name", ".hg"],
                        ["match", "hg-check*"],
                        ["dirname", ".hg"],
                    ],
                ],
                "fields": ["name"],
                "since": res["clock"],
            },
        )
        self.assertEqual(res["clock"]["scm"]["mergebase"], mergeBase)
        self.assertFileListsEqual(res["files"], ["w00t"])

        # and check that subscription results are consistent with it
        self.waitForStatesToVacate(root)
        self.watchmanCommand("flush-subscriptions", root, {"sync_timeout": 1000})
        dat = self.getSubFatClocksOnly("scmsub", root=root)
        self.assertEqual(dat[-1]["clock"]["scm"], res["clock"]["scm"])
        self.assertFileListsEqual(res["files"], self.getConsolidatedFileList(dat))

        # Going back to the merge base, we should get a regular looking incremental
        # list of the files as we would from a since query; we expect to see
        # the removal of w00t and m2
        os.unlink(os.path.join(root, "w00t"))

        self.waitForStatesToVacate(root)
        self.watchmanCommand("flush-subscriptions", root, {"sync_timeout": 1000})
        dat = self.getSubFatClocksOnly("scmsub", root=root)
        self.assertFileListsEqual(["w00t"], self.getConsolidatedFileList(dat))

        self.hg(["co", "-C", "main"], cwd=root)
        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": [
                    "not",
                    [
                        "anyof",
                        ["name", ".hg"],
                        ["match", "hg-check*"],
                        ["dirname", ".hg"],
                    ],
                ],
                "fields": ["name"],
                "since": res["clock"],
            },
        )
        self.assertEqual(res["clock"]["scm"]["mergebase"], mergeBase)
        self.assertFileListsEqual(res["files"], ["w00t", "m2"])

        self.waitForStatesToVacate(root)
        self.watchmanCommand("flush-subscriptions", root, {"sync_timeout": 1000})
        dat = self.getSubFatClocksOnly("scmsub", root=root)
        self.assertEqual(dat[0]["clock"]["scm"], res["clock"]["scm"])
        # we already observed the w00t update above, so we expect to see just the
        # file(s) that changed in the update operation
        self.assertFileListsEqual(["m2"], self.getConsolidatedFileList(dat))

        # Now we're going to move to another branch with a different mergebase.
        self.hg(["co", "-C", "feature1"], cwd=root)
        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": [
                    "not",
                    [
                        "anyof",
                        ["name", ".hg"],
                        ["match", "hg-check*"],
                        ["dirname", ".hg"],
                    ],
                ],
                "fields": ["name"],
                "since": res["clock"],
            },
        )

        # We expect to observe the changed merged base
        self.assertNotEqual(res["clock"]["scm"]["mergebase"], mergeBase)
        # and only the file that changed since that new mergebase
        self.assertFileListsEqual(res["files"], ["a/b/c/f1"])

        # check again that subscription results are consistent with it.
        self.waitForStatesToVacate(root)
        self.watchmanCommand("flush-subscriptions", root, {"sync_timeout": 1000})
        dat = self.getSubFatClocksOnly("scmsub", root=root)
        self.assertEqual(dat[-1]["clock"]["scm"], res["clock"]["scm"])
        # Cookies are written to all the top level directories, thus the
        # set of files will also include the top level directory, even
        # though no files changed in it.
        additionalFiles = ["a"] if watch["watcher"] == "kqueue+fsevents" else []
        self.assertFileListsEqual(
            res["files"] + additionalFiles, self.getConsolidatedFileList(dat)
        )

        # and to check whether our dirstate caching code is reasonable,
        # run a query that should be able to hit the cache
        clock = res["clock"]
        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": [
                    "not",
                    [
                        "anyof",
                        ["name", ".hg"],
                        ["match", "hg-check*"],
                        ["dirname", ".hg"],
                    ],
                ],
                "fields": ["name"],
                "since": {"scm": {"mergebase-with": "main"}},
            },
        )
        self.assertEqual(clock["scm"], res["clock"]["scm"])

        # Fresh instance queries return the complete set of changes (so there is
        # no need to provide information on deleted files). # In contrast, SCM
        # aware queries must contain the deleted files in the result list. Check
        # that the deleted file is part of the result set for feature3.
        self.hg(["co", "-C", "feature3"], cwd=root)
        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": [
                    "not",
                    [
                        "anyof",
                        ["name", ".hg"],
                        ["match", "hg-check*"],
                        ["dirname", ".hg"],
                    ],
                ],
                "fields": ["name"],
                "since": res["clock"],
            },
        )
        self.assertFileListsEqual(
            res["files"], ["a", "a/b", "a/b/c", "a/b/c/f1", "car"]
        )

        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": [
                    "not",
                    [
                        "anyof",
                        ["name", ".hg"],
                        ["match", "hg-check*"],
                        ["dirname", ".hg"],
                    ],
                ],
                "fields": ["name"],
                "since": {"scm": {"mergebase": "", "mergebase-with": "main"}},
            },
        )
        self.assertFileListsEqual(res["files"], ["car"])

        # Make sure that LocalFileResult can render timestamps in the results
        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": ["name", "car"],
                "fields": ["name", "mtime", "atime", "ctime", "content.sha1hex"],
                "since": {"scm": {"mergebase": "", "mergebase-with": "main"}},
            },
        )
        # Since 'car' was deleted, its timestamps are reported as 0
        for ts in ["mtime", "atime", "ctime"]:
            self.assertEqual(res["files"][0][ts], 0)
        self.assertEqual(res["files"][0]["content.sha1hex"], None)

        # Check again with a file that exists
        self.hg(["co", "-C", "feature2"], cwd=root)
        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": ["name", "m2"],
                "fields": ["name", "mtime", "atime", "ctime", "content.sha1hex"],
                "since": {"scm": {"mergebase": "", "mergebase-with": "main"}},
            },
        )
        for ts in ["mtime", "atime", "ctime"]:
            self.assertGreater(res["files"][0][ts], 0)
        self.assertEqual(
            res["files"][0]["content.sha1hex"],
            "da39a3ee5e6b4b0d3255bfef95601890afd80709",
        )

        # Go to the 'initial' bookmark, and query for changes since 'initial'
        # We should ideally not see any changes ...
        self.hg(["co", "-C", "initial"], cwd=root)
        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": ["not", ["anyof", ["name", ".hg"], ["dirname", ".hg"]]],
                "fields": ["name"],
                "since": {"scm": {"mergebase-with": "initial"}},
            },
        )
        self.assertFileListsEqual(res["files"], [])

        # Determine the bookmark hashes
        mergeBaseMain = self.resolveCommitHash("main", cwd=root)
        mergeBaseInitial = self.resolveCommitHash("initial", cwd=root)

        # Checkout initial
        self.hg(["co", "-C", "initial"], cwd=root)
        self.watchmanCommand("flush-subscriptions", root, {"sync_timeout": 1000})
        dat = self.getSubFatClocksOnly("scmsub", root=root)

        # Checkout main - verify merge base change and empty file list
        self.hg(["co", "-C", "main"], cwd=root)

        self.waitForStatesToVacate(root)
        self.watchmanCommand("flush-subscriptions", root, {"sync_timeout": 1000})
        dat = self.getSubFatClocksOnly("scmsub", root=root)
        self.assertEqual(dat[-1]["clock"]["scm"]["mergebase"], mergeBaseMain)
        self.assertFileListsEqual(self.getConsolidatedFileList(dat), [])

        # Checkout initial - verify merge base change and empty file list
        self.hg(["co", "-C", "initial"], cwd=root)

        self.waitForStatesToVacate(root)
        self.watchmanCommand("flush-subscriptions", root, {"sync_timeout": 1000})
        dat = self.getSubFatClocksOnly("scmsub", root=root)
        self.assertEqual(dat[-1]["clock"]["scm"]["mergebase"], mergeBaseInitial)
        self.assertFileListsEqual(self.getConsolidatedFileList(dat), [])

        # Ensure that we reported deleted files correctly, even
        # if we've never seen the files before.  To do this, we're
        # going to cancel the watch and restart it, so this is broken
        # out separately from the earlier tests against feature3
        self.hg(["co", "-C", "feature3"], cwd=root)
        self.watchmanCommand("watch-del", root)
        self.watchmanCommand("watch", root)
        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": [
                    "allof",
                    ["type", "f"],
                    [
                        "not",
                        [
                            "anyof",
                            ["name", ".hg"],
                            ["match", "hg-check*"],
                            ["dirname", ".hg"],
                        ],
                    ],
                ],
                "fields": ["name"],
                "since": {"scm": {"mergebase": "", "mergebase-with": "main"}},
            },
        )
        self.assertFileListsEqual(res["files"], ["car"])

        self.hg(["co", "-C", "feature1"], cwd=root)
        self.watchmanCommand("watch-del", root)
        self.watchmanCommand("watch", root)
        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": [
                    "allof",
                    ["type", "f"],
                    [
                        "not",
                        [
                            "anyof",
                            ["name", ".hg"],
                            ["match", "hg-check*"],
                            ["dirname", ".hg"],
                        ],
                    ],
                ],
                "fields": ["name"],
                "since": {"scm": {"mergebase": "", "mergebase-with": "main"}},
            },
        )
        self.assertFileListsEqual(res["files"], ["a/b/c/f1"])

        # verify behavior of relative_root with a bogus prefix
        res = self.watchmanCommand(
            "query",
            root,
            {
                "relative_root": "bogus",
                "fields": ["name"],
                "since": {"scm": {"mergebase": "", "mergebase-with": "main"}},
            },
        )
        self.assertFileListsEqual(res["files"], [])

        # verify behavior of relative_root with a matching prefix
        res = self.watchmanCommand(
            "query",
            root,
            {
                "relative_root": "a",
                "fields": ["name"],
                "since": {"scm": {"mergebase": "", "mergebase-with": "main"}},
            },
        )
        self.assertFileListsEqual(res["files"], ["b/c/f1"])
