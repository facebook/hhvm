# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import pywatchman
from watchman.integration.lib import WatchmanEdenTestCase


class TestEdenUnmount(WatchmanEdenTestCase.WatchmanEdenTestCase):
    def test_eden_unmount(self) -> None:
        def populate(repo):
            repo.write_file(".watchmanconfig", '{"ignore_dirs":[".buckd"]}')
            repo.write_file("hello", "hola\n")
            repo.commit("initial commit.")

        root = self.makeEdenMount(populate)
        self.watchmanCommand("watch", root)

        clock = self.watchmanCommand("clock", root)
        self.touchRelative(root, "newfile")

        self.eden.unmount(root)

        with self.assertRaises(pywatchman.CommandError) as ctx:
            self.watchmanCommand("query", root, {"fields": ["name"], "since": clock})

        self.assertRegex(str(ctx.exception), "unable to resolve root")
