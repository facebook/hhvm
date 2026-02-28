# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import pywatchman
from watchman.integration.lib import WatchmanTestCase


@WatchmanTestCase.expand_matrix
class TestAuth(WatchmanTestCase.WatchmanTestCase):
    def requiresPersistentSession(self) -> bool:
        return True

    def test_dropPriv(self) -> None:
        root = self.mkdtemp()
        self.touchRelative(root, "111")

        self.watchmanCommand("watch", root)

        # pretend we are not the owner
        self.watchmanCommand("debug-drop-privs")

        # Should be able to watch something that is already watched
        self.watchmanCommand("watch", root)

        # can't make a new watch
        altroot = self.mkdtemp()
        with self.assertRaises(pywatchman.WatchmanError):
            self.watchmanCommand("watch", altroot)

        # Should not be able to delete a watch
        with self.assertRaises(pywatchman.WatchmanError):
            self.watchmanCommand("watch-del", root)

        # or register a trigger
        with self.assertRaises(pywatchman.WatchmanError):
            self.watchmanCommand("trigger", root, "trig", "*.js", "--", "false")
