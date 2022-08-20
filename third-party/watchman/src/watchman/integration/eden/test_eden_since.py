# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.


import json
import os
import shutil
from typing import Any, Dict, Optional

from watchman.integration.lib import WatchmanEdenTestCase


def populate(repo, threshold: Optional[int] = None) -> None:
    # We ignore ".hg" here just so some of the tests that list files don't have to
    # explicitly filter out the contents of this directory.  However, in most situations
    # the .hg directory normally should not be ignored.
    config: Dict[str, Any] = {"ignore_dirs": [".hg"]}
    config["eden_use_streaming_since"] = True
    if threshold:
        config["eden_file_count_threshold_for_fresh_instance"] = threshold
    repo.write_file(".watchmanconfig", json.dumps(config))
    repo.write_file("hello", "hola\n")
    repo.write_file("adir/file", "foo!\n")
    repo.write_file("bdir/test.sh", "#!/bin/bash\necho test\n", mode=0o755)
    repo.write_file("bdir/noexec.sh", "#!/bin/bash\necho test\n")
    repo.symlink("slink", "hello")
    repo.commit("initial commit.")


class TestEdenSince(WatchmanEdenTestCase.WatchmanEdenTestCase):
    def test_eden_lazy_eval(self) -> None:
        root = self.makeEdenMount(populate)
        res = self.watchmanCommand("watch", root)
        self.assertEqual("eden", res["watcher"])

        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": ["allof", ["type", "f"], ["match", "*.sh"]],
                "fields": ["name"],
                "since": "c:0:0",
            },
        )
        self.assertFileListsEqual(res["files"], ["bdir/test.sh", "bdir/noexec.sh"])

    def test_eden_empty_relative_root(self) -> None:
        root = self.makeEdenMount(populate)
        res = self.watchmanCommand("watch", root)
        self.assertEqual("eden", res["watcher"])

        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": ["type", "f"],
                "relative_root": "",
                "fields": ["name"],
                "since": "c:0:0",
            },
        )

        self.assertFileListsEqual(
            res["files"],
            [".watchmanconfig", "hello", "adir/file", "bdir/test.sh", "bdir/noexec.sh"],
        )

    def test_eden_since(self) -> None:
        root = self.makeEdenMount(populate)
        res = self.watchmanCommand("watch", root)
        self.assertEqual("eden", res["watcher"])

        res = self.watchmanCommand(
            "query",
            root,
            {"expression": ["type", "f"], "fields": ["name"], "since": "c:0:0"},
        )
        self.assertTrue(res["is_fresh_instance"])
        self.assertFileListsEqual(
            res["files"],
            ["hello", "adir/file", "bdir/test.sh", "bdir/noexec.sh", ".watchmanconfig"],
        )

        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": ["type", "f"],
                "relative_root": "adir",
                "fields": ["name"],
                "since": "c:0:0",
            },
        )

        self.assertFileListsEqual(
            res["files"],
            ["file"],
            message="should only return adir/file with no adir prefix",
        )

        clock = res["clock"]

        self.touchRelative(root, "hello")
        res = self.watchmanCommand(
            "query",
            root,
            {"expression": ["type", "f"], "fields": ["name"], "since": clock},
        )
        self.assertFileListsEqual(res["files"], ["hello"])

        res = self.watchmanCommand(
            "query",
            root,
            {"expression": ["type", "f"], "fields": ["name", "new"], "since": clock},
        )
        self.assertEqual([{"name": "hello", "new": False}], res["files"])
        self.touchRelative(root, "hello")

        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": ["type", "f"],
                "fields": ["name", "new"],
                "since": res["clock"],
            },
        )
        self.assertEqual([{"name": "hello", "new": False}], res["files"])

        res = self.watchmanCommand(
            "query",
            root,
            {"expression": ["type", "f"], "fields": ["name"], "since": res["clock"]},
        )
        self.assertFileListsEqual(res["files"], [])

        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": ["type", "f"],
                "empty_on_fresh_instance": True,
                "fields": ["name"],
                "since": "c:0:0",
            },
        )
        self.assertTrue(res["is_fresh_instance"])
        self.assertFileListsEqual(res["files"], [])

        os.unlink(os.path.join(root, "hello"))
        res = self.watchmanCommand(
            "query", root, {"fields": ["name"], "since": res["clock"]}
        )
        self.assertFileListsEqual(res["files"], ["hello"])

        res = self.watchmanCommand(
            "query",
            root,
            {"expression": ["type", "f"], "fields": ["name"], "since": res["clock"]},
        )
        self.assertFileListsEqual(res["files"], [])

        self.touchRelative(root, "newfile")
        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": ["type", "f"],
                "fields": ["name", "new"],
                "since": res["clock"],
            },
        )
        self.assertEqual([{"name": "newfile", "new": True}], res["files"])

        self.touchRelative(root, "newfile")
        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": ["type", "f"],
                "fields": ["name", "new"],
                "since": res["clock"],
            },
        )
        self.assertEqual([{"name": "newfile", "new": False}], res["files"])

        adir_file = os.path.join(root, "adir/file")
        os.unlink(adir_file)
        with open(adir_file, "w") as f:
            f.write("new contents\n")
        res = self.watchmanCommand(
            "query",
            root,
            {
                "expression": ["type", "f"],
                "fields": ["name", "new"],
                "since": res["clock"],
            },
        )
        self.assertEqual([{"name": "adir/file", "new": False}], res["files"])

    def query_adir_change_since(self, root, clock):
        return self.watchmanCommand(
            "query",
            root,
            {
                "expression": [
                    "anyof",
                    ["match", "adir", "basename"],
                    ["dirname", "adir"],
                ],
                "fields": ["name", "type"],
                "since": clock,
                "empty_on_fresh_instance": True,
                "always_include_directories": True,
            },
        )

    def test_eden_since_removal(self) -> None:
        root = self.makeEdenMount(populate)

        res = self.watchmanCommand("watch", root)
        self.assertEqual("eden", res["watcher"])

        first_clock = self.watchmanCommand(
            "clock",
            root,
        )["clock"]

        shutil.rmtree(os.path.join(root, "adir"))

        first_res = self.query_adir_change_since(root, first_clock)

        # TODO(T104564495): Watchman reports removed directories as file
        # removal. This is caused by EdenFS's Journal not knowing if a
        # file/directory is removed, and thus this is reported to Watchman as
        # an UNKNOWN file, which for removed files/directory will be reported
        # as a removed file.
        self.assertQueryRepsonseEqual(
            [{"name": "adir", "type": "f"}, {"name": "adir/file", "type": "f"}],
            first_res["files"],
        )

    def test_eden_since_across_update(self) -> None:
        root = self.makeEdenMount(populate)
        repo = self.repoForPath(root)

        res = self.watchmanCommand("watch", root)
        self.assertEqual("eden", res["watcher"])

        shutil.rmtree(os.path.join(root, "adir"))

        # commit the removal so we can test the change across an update.
        repo.hg("addremove")
        repo.commit("removal commit.")

        first_clock = self.watchmanCommand(
            "clock",
            root,
        )["clock"]

        repo.hg("prev")  # add the files back across commits

        first_res = self.query_adir_change_since(root, first_clock)

        self.assertQueryRepsonseEqual(
            [{"name": "adir", "type": "d"}, {"name": "adir/file", "type": "f"}],
            first_res["files"],
        )

        second_clock = self.watchmanCommand(
            "clock",
            root,
        )["clock"]

        repo.hg("next")  # remove the files again across commits

        second_res = self.query_adir_change_since(root, second_clock)

        self.assertQueryRepsonseEqual(
            [{"name": "adir", "type": "d"}, {"name": "adir/file", "type": "f"}],
            second_res["files"],
        )

    def test_eden_since_over_threshold(self) -> None:
        root = self.makeEdenMount(lambda repo: populate(repo, 1))
        repo = self.repoForPath(root)

        res = self.watchmanCommand("watch", root)
        self.assertEqual("eden", res["watcher"])

        clock = self.watchmanCommand(
            "clock",
            root,
        )["clock"]

        def do_query(clock):
            return self.watchmanCommand(
                "query",
                root,
                {
                    "expression": ["type", "f"],
                    "empty_on_fresh_instance": True,
                    "fields": ["name"],
                    "since": clock,
                },
            )

        shutil.rmtree(os.path.join(root, "bdir"))
        repo.hg("addremove")
        repo.commit("removal commit.")

        clock = self.watchmanCommand(
            "clock",
            root,
        )["clock"]

        res = do_query(clock)
        self.assertFalse(res["is_fresh_instance"])
        clock = res["clock"]

        repo.hg("prev")

        # A couple of files changed, more than the threshold one 1 set in the
        # configuration. This is expected to return a fresh instance.
        res = do_query(clock)
        self.assertTrue(res["is_fresh_instance"])
        clock = res["clock"]

        # Make sure that we detect newly edited files afterwards.
        with open(os.path.join(root, "hello"), "w") as f:
            f.write("hello\n")

        res = do_query(clock)
        self.assertFalse(res["is_fresh_instance"])
        self.assertQueryRepsonseEqual(["hello"], res["files"])
