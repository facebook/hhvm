# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import pywatchman
from watchman.integration.lib import WatchmanTestCase


@WatchmanTestCase.expand_matrix
class TestLog(WatchmanTestCase.WatchmanTestCase):
    def test_invalidNumArgsLogLevel(self) -> None:
        for params in [["log-level"], ["log-level", "debug", "extra"]]:
            with self.assertRaises(pywatchman.WatchmanError) as ctx:
                self.watchmanCommand(*params)

            self.assertIn("wrong number of arguments", str(ctx.exception))

    def test_invalidLevelLogLevel(self) -> None:
        with self.assertRaises(pywatchman.WatchmanError) as ctx:
            self.watchmanCommand("log-level", "invalid")

        self.assertIn("invalid log level", str(ctx.exception))

    def test_invalidNumArgsLog(self) -> None:
        for params in [["log"], ["log", "debug"], ["log", "debug", "test", "extra"]]:
            with self.assertRaises(pywatchman.WatchmanError) as ctx:
                self.watchmanCommand(*params)

            self.assertIn("wrong number of arguments", str(ctx.exception))

    def test_invalidLevelLog(self) -> None:
        with self.assertRaises(pywatchman.WatchmanError) as ctx:
            self.watchmanCommand("log", "invalid", "test")

        self.assertIn("invalid log level", str(ctx.exception))
