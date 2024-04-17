# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe

from watchman.integration.lib import WatchmanEdenTestCase


def populate(repo):
    repo.write_file(".watchmanconfig", '{"ignore_dirs":[".buckd"]}')
    repo.write_file("hello", "hola\n")
    repo.commit("initial commit.")


class TestEdenTakeover(WatchmanEdenTestCase.WatchmanEdenTestCase):
    def test_eden_takeover(self) -> None:
        root = self.makeEdenMount(populate)
        self.eden.graceful_restart()

        self.watchmanCommand("watch", root)
        watch_list = self.getWatchList()
        self.assertEqual(len(watch_list), 1)
        self.assertEqual(watch_list[0], root)
