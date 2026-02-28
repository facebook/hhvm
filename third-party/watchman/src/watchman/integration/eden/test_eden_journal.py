# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import time
from os import path

from watchman.integration.lib import WatchmanEdenTestCase


class TestEdenJournal(WatchmanEdenTestCase.WatchmanEdenTestCase):
    def test_eden_journal(self) -> None:
        def populate(repo):
            repo.write_file("hello", "hola\n")
            repo.commit("initial commit.")

        root = self.makeEdenMount(populate)
        repo = self.repoForPath(root)
        initial_commit = repo.get_head_hash()

        res = self.watchmanCommand("watch", root)
        self.assertEqual("eden", res["watcher"])

        clock = self.watchmanCommand("clock", root)

        self.touchRelative(root, "newfile")

        res = self.watchmanCommand("query", root, {"fields": ["name"], "since": clock})
        clock = res["clock"]
        self.assertFileListsEqual(res["files"], ["newfile"])

        repo.add_file("newfile")
        repo.commit(message="add newfile")
        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": [
                    "not",
                    [
                        "anyof",
                        ["dirname", ".hg"],
                        ["match", "checklink*"],
                        ["match", "hg-check*"],
                    ],
                ],
                "fields": ["name"],
                "since": clock,
            },
        )
        clock = res["clock"]
        self.assertFileListsEqual(
            res["files"],
            ["newfile"],
            message="We expect to report the files changed in the commit",
        )

        # Test the the journal has the correct contents across a "reset" like
        # operation where the parents are poked directly.   This is using
        # debugsetparents rather than reset because the latter isn't enabled
        # by default for hg in the watchman test machinery.
        self.touchRelative(root, "unclean")
        repo.hg("debugsetparents", initial_commit)
        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": ["not", ["dirname", ".hg"]],
                "fields": ["name"],
                "since": clock,
            },
        )
        self.assertFileListsEqual(
            res["files"],
            ["newfile", "unclean"],
            message=(
                "We expect to report the file changed in the commit "
                "as well as the unclean file"
            ),
        )

        # make sure that we detect eden getting unmounted.  This sleep is unfortunate
        # and ugly.  Without it, the unmount will fail because something is accessing
        # the filesystem.  I haven't been able to find out what it is because fuser
        # takes too long to run and by the time it has run, whatever that blocker
        # was is not longer there.  Ordinarily I'd prefer to poll on some condition
        # in a loop rather than just sleeping an arbitrary amount, but I just don't
        # know what the offending thing is and running the unmount in a loop is prone
        # to false negatives.
        time.sleep(1)

        eden = self.eden
        assert eden is not None
        eden.remove(root)
        watches = self.watchmanCommand("watch-list")
        self.assertNotIn(root, watches["roots"])

    def test_two_rapid_checkouts_show_briefly_changed_files(self) -> None:
        initial_commit = None
        add_commit = None
        remove_commit = None

        def populate(repo):
            nonlocal initial_commit, add_commit, remove_commit
            repo.write_file("hello", "hola\n")
            initial_commit = repo.commit("initial commit.")

            repo.write_file("newfile", "contents\n")
            add_commit = repo.commit("add newfile")

            repo.remove_file("newfile")
            remove_commit = repo.commit("remove newfile")

        root = self.makeEdenMount(populate)
        repo = self.repoForPath(root)

        # Synchronize to the initial commit.
        repo.update(initial_commit, clean=True)

        res = self.watchmanCommand("watch", root)
        self.assertEqual("eden", res["watcher"])
        clock = self.watchmanCommand("clock", root)
        res = self.watchmanCommand("query", root, {"fields": ["name"], "since": clock})
        clock = res["clock"]

        # Update to the latest commit, through the intermediate.
        repo.update(add_commit)
        repo.update(remove_commit)
        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": [
                    "not",
                    [
                        "anyof",
                        ["dirname", ".hg"],
                        ["match", "checklink*"],
                        ["match", "hg-check*"],
                    ],
                ],
                "fields": ["name", "new"],
                "since": clock,
            },
        )

        res = self.normalizeFiles(res)

        self.assertCountEqual(
            res["files"],
            [{"name": "newfile", "new": False}],
            "Files created and removed across the update operation should show up in the changed list",
        )

    def test_aba_checkouts_show_briefly_changed_files(self) -> None:
        initial_commit = None
        add_commit = None

        def populate(repo):
            nonlocal initial_commit, add_commit
            repo.write_file("hello", "hola\n")
            initial_commit = repo.commit("initial commit.")

            repo.write_file("newfile", "contents\n")
            add_commit = repo.commit("add newfile")

        root = self.makeEdenMount(populate)
        repo = self.repoForPath(root)

        # Synchronize to the initial commit.
        repo.update(initial_commit, clean=True)

        res = self.watchmanCommand("watch", root)
        self.assertEqual("eden", res["watcher"])
        clock = self.watchmanCommand("clock", root)
        res = self.watchmanCommand("query", root, {"fields": ["name"], "since": clock})
        clock = res["clock"]

        # Update to a new file and back to the initial commit, expecting to see the modified file show up in the change list.
        repo.update(add_commit)
        repo.update(initial_commit)
        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": [
                    "not",
                    [
                        "anyof",
                        ["dirname", ".hg"],
                        ["match", "checklink*"],
                        ["match", "hg-check*"],
                    ],
                ],
                "fields": ["name", "new"],
                "since": clock,
            },
        )

        res = self.normalizeFiles(res)

        self.assertCountEqual(
            res["files"],
            [{"name": "newfile", "new": False}],
            "Files created and removed across the update operation should show up in the changed list",
        )

    def test_querying_with_truncated_journal_returns_fresh_instance(self) -> None:
        def populate(repo):
            repo.write_file("hello", "hola\n")
            repo.commit("initial commit.")

        root = self.makeEdenMount(populate)

        res = self.watchmanCommand("watch", root)
        self.assertEqual("eden", res["watcher"])

        clock = self.watchmanCommand("clock", root)

        with self.eden.get_thrift_client_legacy() as thrift_client:
            thrift_client.setJournalMemoryLimit(root, 0)
            self.assertEqual(0, thrift_client.getJournalMemoryLimit(root))

        # Eden's Journal always remembers at least one entry so we will
        # do things in twos

        self.touchRelative(root, "newfile")
        self.touchRelative(root, "newfile2")
        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": [
                    "not",
                    ["anyof", ["dirname", ".hg"], ["dirname", ".eden"]],
                ],
                "fields": ["name"],
                "since": clock,
            },
        )
        clock = res["clock"]
        self.assertTrue(res["is_fresh_instance"])
        self.assertFileListsEqual(
            res["files"], ["hello", "newfile", "newfile2", ".hg", ".eden"]
        )

        self.removeRelative(root, "newfile")
        self.removeRelative(root, "newfile2")
        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": [
                    "not",
                    ["anyof", ["dirname", ".hg"], ["dirname", ".eden"]],
                ],
                "fields": ["name"],
                "since": clock,
            },
        )
        clock = res["clock"]
        self.assertTrue(res["is_fresh_instance"])
        self.assertFileListsEqual(res["files"], ["hello", ".hg", ".eden"])

    def test_changing_root_tree(self) -> None:
        def populate(repo):
            repo.write_file("hello", "hola\n")
            repo.commit("initial commit.")

        root = self.makeEdenMount(populate)

        res = self.watchmanCommand("watch", root)
        self.assertEqual("eden", res["watcher"])

        clock = self.watchmanCommand("clock", root)

        # When the root tree inode changes, EdenFS will report an empty path
        # for such change. This test ensures we handle this case well.
        self.touchRelative(root, "")
        self.touchRelative(root, "")

        # This should not throw
        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": [
                    "not",
                    ["anyof", ["dirname", ".hg"], ["dirname", ".eden"]],
                ],
                "fields": ["name", "exists"],
                "since": clock,
            },
        )

        self.assertEqual(res["files"][0], {"name": path.basename(root), "exists": True})
