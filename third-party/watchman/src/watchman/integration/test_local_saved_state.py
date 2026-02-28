# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import os

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
class TestSavedState(WatchmanSCMTestCase.WatchmanSCMTestCase):
    def checkOSApplicability(self) -> None:
        if is_ubuntu():
            self.skipTest("Test is flaky. See Facebook task T36574087.")
        if "CIRCLECI" in os.environ or "TRAVIS" in os.environ:
            self.skipTest("consistently fails on single core machines!")
        if os.name == "nt":
            self.skipTest("The order of events on Windows is funky")

    def setUp(self) -> None:
        self.skipIfNoFSMonitor()
        self.root = self.mkdtemp()
        """ Set up a repo with a DAG like this:
@  changeset:
|  bookmark:    feature4
|  tag:         tip
|  summary:     add f1
|
| o  changeset:
|/   bookmark:    feature2
|    summary:     remove car
|
o  changeset:
|  bookmark:    main
|  summary:     add bar and car
|
o  changeset:
|  bookmark:    feature3
|  summary:     add m2
|
| o  changeset:
|/   bookmark:    feature1
|    summary:     add m1
|
o  changeset:
|  bookmark:   feature0
|  summary:    add p1
|
o  changeset:
   bookmark:    initial
   summary:     add foo
        """
        self.hg(["init"], cwd=self.root)
        self.touchRelative(self.root, "foo")
        self.hg(["book", "initial"], cwd=self.root)
        self.hg(["addremove"], cwd=self.root)
        self.hg(["commit", "-m", "initial"], cwd=self.root)
        self.hg(["book", "feature0"], cwd=self.root)
        self.touchRelative(self.root, "p1")
        self.hg(["addremove"], cwd=self.root)
        self.hg(["commit", "-m", "add p1"], cwd=self.root)
        self.hg(["book", "feature1"], cwd=self.root)
        self.touchRelative(self.root, "m1")
        self.hg(["addremove"], cwd=self.root)
        self.hg(["commit", "-m", "add m1"], cwd=self.root)
        self.hg(["co", "feature0"], cwd=self.root)
        self.hg(["book", "feature3"], cwd=self.root)
        self.touchRelative(self.root, "m2")
        self.hg(["addremove"], cwd=self.root)
        self.hg(["commit", "-m", "add m2"], cwd=self.root)
        self.hg(["book", "main"], cwd=self.root)
        self.touchRelative(self.root, "bar")
        self.touchRelative(self.root, "car")
        self.hg(["addremove"], cwd=self.root)
        self.hg(["commit", "-m", "add bar and car"], cwd=self.root)
        self.hg(["book", "feature2"], cwd=self.root)
        self.hg(["rm", "car"], cwd=self.root)
        self.hg(["commit", "-m", "remove car"], cwd=self.root)
        self.hg(["co", "main"], cwd=self.root)
        self.hg(["book", "feature4"], cwd=self.root)
        self.touchRelative(self.root, "f1")
        self.hg(["addremove"], cwd=self.root)
        self.hg(["commit", "-m", "add f1"], cwd=self.root)
        self.watchmanCommand("watch", self.root)

    def getQuery(self, config):
        return {
            "expression": [
                "not",
                ["anyof", ["name", ".hg"], ["match", "hg-check*"], ["dirname", ".hg"]],
            ],
            "fields": ["name"],
            "since": {
                "scm": {
                    "mergebase-with": "main",
                    "saved-state": {"storage": "local", "config": config},
                }
            },
        }

    def getLocalFilename(self, saved_state_rev, metadata):
        if metadata:
            return saved_state_rev + "_" + metadata
        return saved_state_rev

    # Creates a saved state (with no content) for the specified project at the
    # specified bookmark within the specified local storage path.
    def saveState(self, project, bookmark, local_storage, metadata=None):
        saved_state_rev = self.resolveCommitHash(bookmark, cwd=self.root)
        project_dir = os.path.join(local_storage, project)
        if not os.path.isdir(project_dir):
            os.mkdir(project_dir)
        filename = self.getLocalFilename(saved_state_rev, metadata)
        self.touchRelative(project_dir, filename)
        return saved_state_rev

    def getConfig(self, result):
        return result["clock"]["scm"]["saved-state"]["config"]

    def assertStorageTypeLocal(self, result) -> None:
        self.assertEqual(result["clock"]["scm"]["saved-state"]["storage"], "local")

    def assertCommitIDEquals(self, result, commit_id) -> None:
        self.assertEqual(result["clock"]["scm"]["saved-state"]["commit-id"], commit_id)

    def assertCommitIDNotPresent(self, result) -> None:
        self.assertTrue("commit-id" not in result["clock"]["scm"]["saved-state"])

    def assertSavedStateErrorEquals(self, result, error_string) -> None:
        self.assertEqual(result["saved-state-info"]["error"], error_string)

    def assertSavedStateInfo(self, result, path, commit_id) -> None:
        self.assertEqual(
            result["saved-state-info"], {"local-path": path, "commit-id": commit_id}
        )

    def assertMergebaseEquals(self, result, mergebase) -> None:
        self.assertEqual(result["clock"]["scm"]["mergebase"], mergebase)

    def test_localSavedStateErrorHandling(self) -> None:
        # Local storage should throw if config does not include
        # local-storage-path. Unit tests more extensively test all possible
        # error cases, this just confirms that an example error propagates end
        # to end properly.
        config = {"project": "test"}
        test_query = self.getQuery(config)
        with self.assertRaises(pywatchman.WatchmanError) as ctx:
            self.watchmanCommand("query", self.root, test_query)
        self.assertIn(
            "'local-storage-path' must be present in saved state config",
            str(ctx.exception),
        )

    def test_localSavedStateNoStateFound(self) -> None:
        # Local saved state should return no commit id and error message if no
        # valid state found (project does not match states saved above)
        local_storage = self.mkdtemp()
        self.saveState("example_project", "feature3", local_storage)
        self.saveState("example_project", "feature0", local_storage)
        config = {"local-storage-path": local_storage, "project": "does-not-exist"}
        test_query = self.getQuery(config)
        res = self.watchmanCommand("query", self.root, test_query)
        expected_mergebase = self.resolveCommitHash("main", cwd=self.root)
        self.assertMergebaseEquals(res, expected_mergebase)
        self.assertCommitIDNotPresent(res)
        self.assertEqual(self.getConfig(res), config)
        self.assertStorageTypeLocal(res)
        self.assertSavedStateErrorEquals(res, "No suitable saved state found")
        self.assertFileListsEqual(res["files"], ["foo", "p1", "m2", "bar", "car", "f1"])

    def test_localSavedStateNotWithinLimit(self) -> None:
        # Local saved state should return an empty commit id, error message,
        # and changed files since prior clock if the first available saved
        # state is not within the limit
        local_storage = self.mkdtemp()
        self.saveState("example_project", "feature3", local_storage)
        self.saveState("example_project", "feature0", local_storage)
        config = {
            "local-storage-path": local_storage,
            "project": "example_project",
            "max-commits": 1,
        }
        test_query = self.getQuery(config)
        res = self.watchmanCommand("query", self.root, test_query)
        self.assertSavedStateErrorEquals(res, "No suitable saved state found")
        self.assertEqual(self.getConfig(res), config)
        self.assertStorageTypeLocal(res)
        expected_mergebase = self.resolveCommitHash("main", cwd=self.root)
        self.assertMergebaseEquals(res, expected_mergebase)
        self.assertFileListsEqual(res["files"], ["foo", "p1", "m2", "bar", "car", "f1"])

    def test_localSavedStateNotWithinLimitOmitChangedFiles(self) -> None:
        # Local saved state should return an empty commit id, error message,
        # and changed files since prior clock if the first available saved
        # state is not within the limit
        local_storage = self.mkdtemp()
        self.saveState("example_project", "feature3", local_storage)
        self.saveState("example_project", "feature0", local_storage)
        config = {
            "local-storage-path": local_storage,
            "project": "example_project",
            "max-commits": 1,
        }
        test_query = self.getQuery(config)
        test_query["omit_changed_files"] = True
        res = self.watchmanCommand("query", self.root, test_query)
        self.assertSavedStateErrorEquals(res, "No suitable saved state found")
        self.assertEqual(self.getConfig(res), config)
        self.assertStorageTypeLocal(res)
        expected_mergebase = self.resolveCommitHash("main", cwd=self.root)
        self.assertMergebaseEquals(res, expected_mergebase)
        self.assertFileListsEqual(res["files"], [])

    def test_localSavedStateNotWithinLimitError(self) -> None:
        # Local saved state should return an empty commit id, error message,
        # and changed files since prior clock if the first available saved
        # state is not within the limit
        local_storage = self.mkdtemp()
        self.saveState("example_project", "feature3", local_storage)
        self.saveState("example_project", "feature0", local_storage)
        config = {
            "local-storage-path": local_storage,
            "project": "example_project",
            "max-commits": 1,
        }
        test_query = self.getQuery(config)
        test_query["fail_if_no_saved_state"] = True
        with self.assertRaises(pywatchman.WatchmanError) as ctx:
            self.watchmanCommand("query", self.root, test_query)
        self.assertIn(
            (
                "The merge base changed but no corresponding saved state "
                "was found for the new merge base"
            ),
            str(ctx.exception),
        )

    def test_localSavedStateLookupSuccess(self) -> None:
        # Local saved state should return the saved state commit id, info, and
        # changed files since the saved state if valid state found within limit
        local_storage = self.mkdtemp()
        saved_state_rev_feature3 = self.saveState(
            "example_project", "feature3", local_storage
        )
        self.saveState("example_project", "feature0", local_storage)
        config = {
            "local-storage-path": local_storage,
            "project": "example_project",
            "max-commits": 10,
        }
        test_query = self.getQuery(config)
        res = self.watchmanCommand("query", self.root, test_query)
        expected_mergebase = self.resolveCommitHash("main", cwd=self.root)
        self.assertMergebaseEquals(res, expected_mergebase)
        self.assertStorageTypeLocal(res)
        self.assertCommitIDEquals(res, saved_state_rev_feature3)
        self.assertEqual(self.getConfig(res), config)
        project_dir = os.path.join(local_storage, "example_project")
        expected_path = os.path.join(project_dir, saved_state_rev_feature3)
        self.assertSavedStateInfo(res, expected_path, saved_state_rev_feature3)
        self.assertFileListsEqual(res["files"], ["f1", "bar", "car"])

    def test_localSavedStateLookupSuccessOmitChangedFiles(self) -> None:
        # Local saved state should return the saved state commit id, info, and
        # changed files since the saved state if valid state found within limit.
        # Since this sets omit_changed_files, we only return the files from the
        # saved-state to the mergebase
        local_storage = self.mkdtemp()
        saved_state_rev_feature3 = self.saveState(
            "example_project", "feature3", local_storage
        )
        self.saveState("example_project", "feature0", local_storage)
        config = {
            "local-storage-path": local_storage,
            "project": "example_project",
            "max-commits": 10,
        }
        test_query = self.getQuery(config)
        test_query["omit_changed_files"] = True
        res = self.watchmanCommand("query", self.root, test_query)
        expected_mergebase = self.resolveCommitHash("main", cwd=self.root)
        self.assertMergebaseEquals(res, expected_mergebase)
        self.assertStorageTypeLocal(res)
        self.assertCommitIDEquals(res, saved_state_rev_feature3)
        self.assertEqual(self.getConfig(res), config)
        project_dir = os.path.join(local_storage, "example_project")
        expected_path = os.path.join(project_dir, saved_state_rev_feature3)
        self.assertSavedStateInfo(res, expected_path, saved_state_rev_feature3)
        self.assertFileListsEqual(res["files"], [])

    def test_localSavedStateLookupSuccessWithMetadata(self) -> None:
        local_storage = self.mkdtemp()
        metadata = "metadata"
        saved_state_rev_feature3 = self.saveState(
            "example_project", "feature3", local_storage, metadata
        )
        self.saveState("example_project", "feature0", local_storage, metadata)
        config = {
            "local-storage-path": local_storage,
            "project": "example_project",
            "project-metadata": metadata,
            "max-commits": 10,
        }
        test_query = self.getQuery(config)
        res = self.watchmanCommand("query", self.root, test_query)
        expected_mergebase = self.resolveCommitHash("main", cwd=self.root)
        self.assertMergebaseEquals(res, expected_mergebase)
        self.assertStorageTypeLocal(res)
        self.assertCommitIDEquals(res, saved_state_rev_feature3)
        self.assertEqual(self.getConfig(res), config)
        project_dir = os.path.join(local_storage, "example_project")
        filepath = self.getLocalFilename(saved_state_rev_feature3, metadata)
        expected_path = os.path.join(project_dir, filepath)
        self.assertSavedStateInfo(res, expected_path, saved_state_rev_feature3)
        self.assertFileListsEqual(res["files"], ["f1", "bar", "car"])

    def test_localSavedStateFailureIfMetadataDoesNotMatch(self) -> None:
        local_storage = self.mkdtemp()
        self.saveState("example_project", "feature3", local_storage)
        self.saveState("example_project", "feature0", local_storage)
        config = {
            "local-storage-path": local_storage,
            "project": "example_project",
            "project-metadata": "meta",
            "max-commits": 10,
        }
        test_query = self.getQuery(config)
        res = self.watchmanCommand("query", self.root, test_query)
        self.assertSavedStateErrorEquals(res, "No suitable saved state found")
        self.assertEqual(self.getConfig(res), config)
        self.assertStorageTypeLocal(res)
        expected_mergebase = self.resolveCommitHash("main", cwd=self.root)
        self.assertMergebaseEquals(res, expected_mergebase)
        self.assertFileListsEqual(res["files"], ["foo", "p1", "m2", "bar", "car", "f1"])

    def test_localSavedStateFailureIfNoMetadataForFileThatHasIt(self) -> None:
        local_storage = self.mkdtemp()
        metadata = "metadata"
        self.saveState("example_project", "feature3", local_storage, metadata)
        self.saveState("example_project", "feature0", local_storage, metadata)
        config = {
            "local-storage-path": local_storage,
            "project": "example_project",
            "max-commits": 10,
        }
        test_query = self.getQuery(config)
        res = self.watchmanCommand("query", self.root, test_query)
        self.assertSavedStateErrorEquals(res, "No suitable saved state found")
        self.assertEqual(self.getConfig(res), config)
        self.assertStorageTypeLocal(res)
        expected_mergebase = self.resolveCommitHash("main", cwd=self.root)
        self.assertMergebaseEquals(res, expected_mergebase)
        self.assertFileListsEqual(res["files"], ["foo", "p1", "m2", "bar", "car", "f1"])

    def test_localSavedStateSubscription(self) -> None:
        local_storage = self.mkdtemp()
        saved_state_rev_feature3 = self.saveState(
            "example_project", "feature3", local_storage
        )
        saved_state_rev_feature0 = self.saveState(
            "example_project", "feature0", local_storage
        )
        # Set up a subscription for the successful query and confirm that the
        # subscription response contains the most recent saved state to the
        # current rev's mergebase, and the changed files since that rev
        config = {
            "local-storage-path": local_storage,
            "project": "example_project",
            "max-commits": 10,
        }
        test_query = self.getQuery(config)
        sub = self.watchmanCommand("subscribe", self.root, "scmsub", test_query)
        self.waitForStatesToVacate(self.root)
        syncTimeout = {"sync_timeout": 1000}
        self.watchmanCommand("flush-subscriptions", self.root, syncTimeout)
        dat = self.getSubFatClocksOnly("scmsub", root=self.root)
        expected_mergebase = self.resolveCommitHash("main", cwd=self.root)
        self.assertMergebaseEquals(sub, expected_mergebase)
        self.assertStorageTypeLocal(sub)
        self.assertEqual(self.getConfig(sub), config)
        self.assertCommitIDEquals(sub, saved_state_rev_feature3)
        project_dir = os.path.join(local_storage, "example_project")
        expected_path = os.path.join(project_dir, saved_state_rev_feature3)
        self.assertSavedStateInfo(sub, expected_path, saved_state_rev_feature3)
        self.assertFileListsEqual(
            self.getConsolidatedFileList(dat), ["f1", "bar", "car"]
        )
        # Check out a rev with the same merge base and confirm saved state
        # commit id is unchanged, info is not present, and file list is updated
        self.hg(["co", "-C", "feature2"], cwd=self.root)
        self.waitForStatesToVacate(self.root)
        self.watchmanCommand("flush-subscriptions", self.root, syncTimeout)
        dat = self.getSubFatClocksOnly("scmsub", root=self.root)
        self.assertFileListsEqual(self.getConsolidatedFileList(dat), ["f1", "car"])
        last_res = dat[-1]
        self.assertTrue("saved-state-info" not in last_res)
        self.assertStorageTypeLocal(last_res)
        self.assertEqual(self.getConfig(last_res), config)
        self.assertMergebaseEquals(last_res, expected_mergebase)
        self.assertCommitIDEquals(last_res, saved_state_rev_feature3)
        # Check out rev with a different mergebase and confirm mergebase
        # changes, new saved state info is returned, and file list is relative
        # to the new mergebase
        self.hg(["co", "-C", "feature1"], cwd=self.root)
        self.waitForStatesToVacate(self.root)
        self.watchmanCommand("flush-subscriptions", self.root, syncTimeout)
        dat = self.getSubFatClocksOnly("scmsub", root=self.root)
        self.assertFileListsEqual(self.getConsolidatedFileList(dat), ["m1"])
        last_res = dat[-1]
        self.assertStorageTypeLocal(last_res)
        self.assertEqual(self.getConfig(last_res), config)
        expected_path = os.path.join(project_dir, saved_state_rev_feature0)
        self.assertSavedStateInfo(last_res, expected_path, saved_state_rev_feature0)
        expected_mergebase = self.resolveCommitHash("feature0", cwd=self.root)
        self.assertMergebaseEquals(last_res, expected_mergebase)
        self.assertCommitIDEquals(last_res, saved_state_rev_feature0)
        # Switch to a commit with saved state for that actual commit and ensure
        # file list is empty
        self.hg(["co", "-C", "feature3"], cwd=self.root)
        self.waitForStatesToVacate(self.root)
        self.watchmanCommand("flush-subscriptions", self.root, syncTimeout)
        dat = self.getSubFatClocksOnly("scmsub", root=self.root)
        self.assertFileListsEqual(self.getConsolidatedFileList(dat), [])
        last_res = dat[-1]
        self.assertStorageTypeLocal(last_res)
        self.assertEqual(self.getConfig(last_res), config)
        expected_path = os.path.join(project_dir, saved_state_rev_feature3)
        self.assertSavedStateInfo(last_res, expected_path, saved_state_rev_feature3)
        expected_mergebase = self.resolveCommitHash("feature3", cwd=self.root)
        self.assertMergebaseEquals(last_res, expected_mergebase)
        self.assertCommitIDEquals(last_res, saved_state_rev_feature3)
        # Make sure that without any saved state available we get a saved state
        # error message, no commit ID is specified, but the subscription
        # otherwise succeeds, and we get all changes since the prior clock
        self.hg(["co", "-C", "initial"], cwd=self.root)
        self.waitForStatesToVacate(self.root)
        self.watchmanCommand("flush-subscriptions", self.root, syncTimeout)
        dat = self.getSubFatClocksOnly("scmsub", root=self.root)
        self.assertFileListsEqual(self.getConsolidatedFileList(dat), ["m2", "p1"])
        last_res = dat[-1]
        self.assertStorageTypeLocal(last_res)
        self.assertEqual(self.getConfig(last_res), config)
        self.assertSavedStateErrorEquals(last_res, "No suitable saved state found")
        expected_mergebase = self.resolveCommitHash("initial", cwd=self.root)
        self.assertMergebaseEquals(last_res, expected_mergebase)
        self.assertCommitIDNotPresent(last_res)
