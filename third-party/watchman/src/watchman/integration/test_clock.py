# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


from watchman.integration.lib import WatchmanTestCase


@WatchmanTestCase.expand_matrix
class TestClock(WatchmanTestCase.WatchmanTestCase):
    def test_clock(self) -> None:
        root = self.mkdtemp()
        self.watchmanCommand("watch", root)
        clock = self.watchmanCommand("clock", root)

        self.assertRegex(clock["clock"], "^c:\\d+:\\d+:\\d+:\\d+$")

    def test_clock_sync(self) -> None:
        root = self.mkdtemp()
        self.watchmanCommand("watch", root)
        clock1 = self.watchmanCommand("clock", root, {"sync_timeout": 5000})
        self.assertRegex(clock1["clock"], "^c:\\d+:\\d+:\\d+:\\d+$")

        clock2 = self.watchmanCommand("clock", root, {"sync_timeout": 5000})
        self.assertRegex(clock2["clock"], "^c:\\d+:\\d+:\\d+:\\d+$")

        self.assertNotEqual(clock1, clock2)
