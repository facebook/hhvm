# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import json
import os
import os.path

from watchman.integration.lib import WatchmanTestCase


@WatchmanTestCase.expand_matrix
class TestSince(WatchmanTestCase.WatchmanTestCase):
    def test_sinceIssue1(self) -> None:
        root = self.mkdtemp()
        self.touchRelative(root, "111")
        self.touchRelative(root, "222")

        self.watchmanCommand("watch", root)
        self.assertFileList(root, ["111", "222"])

        # Create a cursor for this state
        self.watchmanCommand("since", root, "n:foo")

        bar_dir = os.path.join(root, "bar")
        os.mkdir(bar_dir)
        self.touchRelative(bar_dir, "333")
        self.waitForSync(root)

        # We should not observe 111 or 222
        self.assertFileList(root, cursor="n:foo", files=["bar", "bar/333"])

    def test_sinceIssue2(self) -> None:
        root = self.mkdtemp()
        watch = self.watchmanCommand("watch", root)
        self.assertFileList(root, files=[])

        foo_dir = os.path.join(root, "foo")
        os.mkdir(foo_dir)
        self.touchRelative(foo_dir, "111")
        self.waitForSync(root)

        self.assertFileList(root, cursor="n:foo", files=["foo", "foo/111"])

        bar_dir = os.path.join(foo_dir, "bar")
        os.mkdir(bar_dir)
        self.touchRelative(bar_dir, "222")

        # wait until we observe all the files
        self.assertFileList(root, files=["foo", "foo/111", "foo/bar", "foo/bar/222"])

        # now check the delta for the since
        expected = ["foo/bar", "foo/bar/222"]
        files = self.getFileList(root, cursor="n:foo")
        if watch["watcher"] in (
            "inotify",
            "portfs",
            "kqueue",
            "fsevents",
            "dirfsevents",
            "kqueue+fsevents",
        ):
            # These systems also show the containing dir as modified
            expected.append("foo")
        elif watch["watcher"] == "win32":
            # the containing directory sometimes(!) shows as modified
            # on win32, but the important thing is that the other files
            # show up in the list
            files = [f for f in files if f != "foo"]
        self.assertFileListsEqual(files, expected)

    def test_sinceRelativeRoot(self) -> None:
        root = self.mkdtemp()
        self.watchmanCommand("watch", root)
        clock = self.watchmanCommand("clock", root)["clock"]

        self.touchRelative(root, "a")
        os.mkdir(os.path.join(root, "subdir"))
        self.touchRelative(os.path.join(root, "subdir"), "foo")
        self.assertFileList(root, files=["a", "subdir", "subdir/foo"])

        _c, start_time, pid, root_number, ticks = clock.split(":")
        # Move the clock back to avoid flakiness when mkdir/touch are fast enough to make the
        # query have the same time as `clock`
        before_clock = "c:{}:{}:{}:{}".format(
            str(int(start_time) - 1), pid, root_number, ticks
        )
        res = self.watchmanCommand(
            "query",
            root,
            {"since": before_clock, "relative_root": "subdir", "fields": ["name"]},
        )
        self.assertFileListsEqual(res["files"], ["foo"])

        # touch a file outside the relative root
        self.touchRelative(root, "b")
        self.assertFileList(root, files=["a", "b", "subdir", "subdir/foo"])
        res = self.watchmanCommand(
            "query",
            root,
            {"since": res["clock"], "relative_root": "subdir", "fields": ["name"]},
        )
        expect = []
        # Filter out 'foo' as some operating systems may report
        # it and others may not.  We're not interested in it here.
        self.assertFileListsEqual(filter(lambda x: x != "foo", res["files"]), expect)

        # touching just the subdir shouldn't cause anything to show up
        self.touchRelative(root, "subdir")
        self.waitForSync(root)
        res = self.watchmanCommand(
            "query",
            root,
            {"since": res["clock"], "relative_root": "subdir", "fields": ["name"]},
        )
        self.assertFileListsEqual(res["files"], [])

        # touching a new file inside the subdir should cause it to show up
        dir2 = os.path.join(root, "subdir", "dir2")
        os.mkdir(dir2)
        self.touchRelative(dir2, "bar")
        self.waitForSync(root)
        res = self.watchmanCommand(
            "query",
            root,
            {"since": res["clock"], "relative_root": "subdir", "fields": ["name"]},
        )
        self.assertFileListsEqual(res["files"], ["dir2", "dir2/bar"])

    def assertFreshInstanceForSince(self, root, cursor, empty: bool = False) -> None:
        res = self.watchmanCommand(
            "query",
            root,
            {"since": cursor, "fields": ["name"], "empty_on_fresh_instance": empty},
        )
        self.assertTrue(res["is_fresh_instance"])
        if empty:
            self.assertFileListsEqual(res["files"], [])
        else:
            self.assertFileListsEqual(res["files"], ["111"])

    def test_sinceFreshInstance(self) -> None:
        root = self.mkdtemp()
        self.watchmanCommand("watch", root)
        self.assertFileList(root, [])
        self.touchRelative(root, "111")

        res = self.watchmanCommand("query", root, {"fields": ["name"]})
        self.assertTrue(res["is_fresh_instance"])
        self.assertFileListsEqual(res["files"], ["111"])

        # relative clock value, fresh instance
        self.assertFreshInstanceForSince(root, "c:0:1:0:1", False)

        # old-style clock value (implies fresh instance, event if the
        # pid is the same)
        pid = self.watchmanCommand("get-pid")["pid"]
        self.assertFreshInstanceForSince(root, "c:%s:1" % pid, False)

        # -- decompose clock and replace elements one by one
        clock = self.watchmanCommand("clock", root)["clock"]
        p = clock.split(":")
        # ['c', startTime, pid, rootNum, ticks]
        self.assertEqual(len(p), 5)

        # replace start time
        self.assertFreshInstanceForSince(
            root, ":".join(["c", "0", p[2], p[3], p[4]]), False
        )

        # replace pid
        self.assertFreshInstanceForSince(
            root, ":".join(["c", p[1], "1", p[3], p[4]]), False
        )

        # replace root number (also try empty_on_fresh_instance)
        self.assertFreshInstanceForSince(
            root, ":".join(["c", p[1], p[2], "0", p[4]]), True
        )

        # empty_on_fresh_instance, not a fresh instance
        self.touchRelative(root, "222")
        res = self.watchmanCommand(
            "query",
            root,
            {"since": clock, "fields": ["name"], "empty_on_fresh_instance": True},
        )
        self.assertFalse(res["is_fresh_instance"])
        self.assertFileListsEqual(res["files"], ["222"])

        # fresh instance results should omit deleted files
        os.unlink(os.path.join(root, "111"))
        res = self.watchmanCommand(
            "query", root, {"since": "c:0:1:0:1", "fields": ["name"]}
        )
        self.assertTrue(res["is_fresh_instance"])
        self.assertFileListsEqual(res["files"], ["222"])

    def test_reAddWatchFreshInstance(self) -> None:
        root = self.mkdtemp()
        self.watchmanCommand("watch", root)
        self.assertFileList(root, [])
        self.touchRelative(root, "111")

        res = self.watchmanCommand("query", root, {"fields": ["name"]})
        self.assertTrue(res["is_fresh_instance"])
        self.assertFileListsEqual(res["files"], ["111"])

        clock = res["clock"]
        os.unlink(os.path.join(root, "111"))
        self.watchmanCommand("watch-del", root)

        self.watchmanCommand("watch", root)
        self.touchRelative(root, "222")

        # wait for touch to be observed
        self.assertFileList(root, ["222"])

        # ensure that our since query is a fresh instance
        res = self.watchmanCommand("query", root, {"since": clock, "fields": ["name"]})
        self.assertTrue(res["is_fresh_instance"])
        self.assertFileListsEqual(res["files"], ["222"])

    def test_recrawlFreshInstance(self) -> None:
        root = self.mkdtemp()
        self.watchmanCommand("watch", root)
        self.touchRelative(root, "111")
        self.assertFileList(root, ["111"])

        res = self.watchmanCommand("query", root, {"fields": ["name"]})
        self.assertTrue(res["is_fresh_instance"])

        clock = res["clock"]
        os.unlink(os.path.join(root, "111"))
        self.watchmanCommand("debug-recrawl", root)

        self.touchRelative(root, "222")
        res = self.watchmanCommand("query", root, {"since": clock, "fields": ["name"]})
        # In earlier versions of the server, the recrawl would always
        # generate a fresh instance result set.  This is no longer true.
        self.assertFalse(res["is_fresh_instance"])
        self.assertFileListsEqual(res["files"], ["111", "222"])
        self.assertRegex(res["warning"], "Recrawled this watch")

    def test_recrawlFreshInstanceWarningSuppressed(self) -> None:
        root = self.mkdtemp()
        with open(os.path.join(root, ".watchmanconfig"), "w") as f:
            f.write(json.dumps({"suppress_recrawl_warnings": True}))

        self.watchmanCommand("watch", root)
        self.touchRelative(root, "111")
        self.assertFileList(root, [".watchmanconfig", "111"])

        res = self.watchmanCommand("query", root, {"fields": ["name"]})
        self.assertTrue(res["is_fresh_instance"])

        clock = res["clock"]
        os.unlink(os.path.join(root, "111"))
        self.watchmanCommand("debug-recrawl", root)

        self.touchRelative(root, "222")
        res = self.watchmanCommand("query", root, {"since": clock, "fields": ["name"]})
        # In earlier versions of the server, the recrawl would always
        # generate a fresh instance result set.  This is no longer true.
        self.assertFalse(res["is_fresh_instance"])
        self.assertFileListsEqual(res["files"], ["111", "222"])
        self.assertTrue("warning" not in res)
