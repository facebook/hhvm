# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import os

from watchman.integration.lib import WatchmanEdenTestCase


def possible_cookie(name):
    return ".watchman-cookie-" in name


class TestEdenSubscribe(WatchmanEdenTestCase.WatchmanEdenTestCase):
    def requiresPersistentSession(self) -> bool:
        return True

    def test_eden_subscribe(self) -> None:
        commits = []

        def populate(repo):
            # We ignore ".hg" here just so some of the tests that list files don't have
            # to explicitly filter out the contents of this directory.  However, in most
            # situations the .hg directory normally should not be ignored.
            repo.write_file(".watchmanconfig", '{"ignore_dirs":[".buckd", ".hg"]}')
            repo.write_file("hello", "hola\n")
            commits.append(repo.commit("initial commit."))
            repo.write_file("welcome", "bienvenue\n")
            commits.append(repo.commit("commit 2"))
            repo.write_file("readme.txt", "important docs\n")
            commits.append(repo.commit("commit 3"))
            # Switch back to the first commit at the start of the test
            repo.update(commits[0])

        root = self.makeEdenMount(populate)
        repo = self.repoForPath(root)

        res = self.watchmanCommand("watch", root)
        self.assertEqual("eden", res["watcher"])

        self.watchmanCommand(
            "subscribe",
            root,
            "myname",
            {"fields": ["name"], "expression": ["not", ["match", ".watchman-cookie*"]]},
        )

        dat = self.waitForSub("myname", root=root)[0]
        self.assertTrue(dat["is_fresh_instance"])
        self.assertFileListsEqual(
            dat["files"], self.eden_dir_entries + [".eden", ".watchmanconfig", "hello"]
        )

        self.touchRelative(root, "w0000t")
        dat = self.waitForSub("myname", root=root)[0]
        self.assertEqual(False, dat["is_fresh_instance"])
        self.assertFileListsEqual(dat["files"], ["w0000t"])

        # we should not observe .buckd in the subscription results
        # because it is listed in the ignore_dirs config section.
        os.mkdir(os.path.join(root, ".buckd"))

        self.touchRelative(root, "hello")
        dat = self.waitForSub("myname", root=root)[0]
        self.assertEqual(False, dat["is_fresh_instance"])
        self.assertFileListsEqual(dat["files"], ["hello"])

        # performing an hg checkout should notify us of the files changed between
        # commits
        repo.update(commits[2])
        dat = self.waitForSub("myname", root=root)[0]
        self.assertEqual(False, dat["is_fresh_instance"])
        self.assertFileListsEqual(dat["files"], ["welcome", "readme.txt"])

        # make another subscription and assert that we get a fresh
        # instance result with all the files in it
        self.watchmanCommand(
            "subscribe",
            root,
            "othersub",
            {"fields": ["name"], "expression": ["not", ["match", ".watchman-cookie*"]]},
        )

        dat = self.waitForSub("othersub", root=root)[0]
        self.assertEqual(True, dat["is_fresh_instance"])
        self.assertFileListsEqual(
            dat["files"],
            self.eden_dir_entries
            + [".eden", ".watchmanconfig", "hello", "w0000t", "welcome", "readme.txt"],
        )

    def assertWaitForAssertedStates(self, root, states) -> None:
        def sortStates(states):
            """Deterministically sort the states for comparison.
            We sort by name and rely on the sort being stable as the
            relative ordering of the potentially multiple queueued
            entries per name is important to preserve"""
            return sorted(states, key=lambda x: x["name"])

        states = sortStates(states)

        def getStates():
            res = self.watchmanCommand("debug-get-asserted-states", root)
            return sortStates(res["states"])

        self.assertWaitForEqual(states, getStates)

    def test_state_enter_leave(self) -> None:
        """Check that state-enter and state-leave are basically working.
        This is a subset of the tests that are performed in test_subscribe.py;
        we only strictly need to check the basic plumbing here and need not
        replicate the entire set of tests"""

        def populate(repo):
            repo.write_file("hello", "hola\n")
            repo.commit("initial commit.")

        root = self.makeEdenMount(populate)
        res = self.watchmanCommand("watch", root)
        self.assertEqual("eden", res["watcher"])

        result = self.watchmanCommand("debug-get-asserted-states", root)
        self.assertEqual([], result["states"])

        self.watchmanCommand("state-enter", root, "foo")
        self.watchmanCommand("state-enter", root, "bar")
        self.assertWaitForAssertedStates(
            root,
            [
                {"name": "bar", "state": "Asserted"},
                {"name": "foo", "state": "Asserted"},
            ],
        )

        self.watchmanCommand("state-leave", root, "foo")
        self.assertWaitForAssertedStates(root, [{"name": "bar", "state": "Asserted"}])

        self.watchmanCommand("state-leave", root, "bar")
        self.assertWaitForAssertedStates(root, [])
