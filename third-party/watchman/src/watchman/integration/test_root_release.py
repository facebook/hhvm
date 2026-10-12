# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import os
import subprocess
import sys

import pywatchman
from watchman.integration.lib import WatchmanTestCase


@WatchmanTestCase.expand_matrix
class TestRootRelease(WatchmanTestCase.WatchmanTestCase):
    """
    A root that is no longer watched must actually be destroyed, so that
    the view, the watcher and its descriptors are released. The daemon
    logs `root: final ref on <path>` from `~Root`, and the test harness
    runs it with `--log-level=2`, so that line is the observable signal.
    """

    def makeRoot(self) -> str:
        # The subdirectory matters: the kqueue+fsevents watcher only creates
        # an FSEventsWatcher for top-level directories.
        root = self.mkdtemp()
        self.touchRelative(root, "file")
        os.mkdir(os.path.join(root, "sub"))
        self.touchRelative(root, "sub", "file")
        return root

    def watchRoot(self, root: str) -> str:
        """
        Watches and crawls `root` and returns the path as the daemon spells
        it, which is also how `~Root` logs it.
        """
        log_lines_before = len(self.getServerLogContents())
        try:
            watched = self.watchmanCommand("watch", root)["watch"]
            self.assertFileList(root, ["file", "sub", "sub/file"])
        except (pywatchman.CommandError, AssertionError):
            # Sandcastle Mac VMs boot with fseventsd disabled, so no fsevents
            # watcher can start there: `watch` fails, or the crawl cancels
            # the root when kqueue+fsevents fails to start one for `sub`.
            if any(
                "FSEventStreamStart failed" in line
                for line in self.getServerLogContents()[log_lines_before:]
            ):
                self.skipTest("FSEvents is unavailable on this host")
            raise
        return watched

    def assertRootReleased(self, watched: str) -> None:
        needle = "root: final ref on %s" % watched
        self.assertWaitFor(
            lambda: any(needle in line for line in self.getServerLogContents()),
            message="%s was not destroyed after it stopped being watched" % watched,
        )

    def daemonDescriptorCount(self) -> int:
        pid = self.watchmanInstance().pid
        if sys.platform == "linux":
            return len(os.listdir("/proc/%d/fd" % pid))
        if sys.platform == "darwin":
            out = subprocess.check_output(
                ["lsof", "-n", "-P", "-F", "f", "-p", str(pid)], text=True
            )
            return sum(1 for line in out.splitlines() if line[1:].isdigit())
        self.skipTest("no way to count the daemon's descriptors on this platform")

    def test_watch_del_releases_root(self) -> None:
        root = self.makeRoot()
        watched = self.watchRoot(root)

        self.watchmanCommand("watch-del", root)
        self.assertFalse(self.rootIsWatched(root))
        self.assertRootReleased(watched)

    def test_removed_root_is_released(self) -> None:
        root = self.makeRoot()
        watched = self.watchRoot(root)

        os.unlink(os.path.join(root, "sub", "file"))
        os.rmdir(os.path.join(root, "sub"))
        os.unlink(os.path.join(root, "file"))
        os.rmdir(root)
        self.assertWaitFor(
            lambda: not self.rootIsWatched(root),
            message="%s should be cancelled" % root,
        )
        self.assertRootReleased(watched)

    def test_watch_del_releases_descriptors(self) -> None:
        def cycle() -> None:
            root = self.makeRoot()
            watched = self.watchRoot(root)
            self.watchmanCommand("watch-del", root)
            self.assertRootReleased(watched)

        # The first root may allocate state the daemon keeps for its lifetime.
        cycle()
        baseline = self.daemonDescriptorCount()
        for _ in range(5):
            cycle()
        # A client connection the daemon is still closing can briefly count.
        self.assertWaitFor(
            lambda: self.daemonDescriptorCount() <= baseline,
            message="the daemon holds more descriptors than the %d it had before "
            "watching and unwatching 5 roots" % baseline,
        )
