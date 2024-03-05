# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import os
from typing import List

import pywatchman
from watchman.integration.lib import WatchmanTestCase


def populate_tests(original_test_class):
    # Rather than one long sequential test that tests every buffer size, register
    # an individual test per size. This allows more tests to run in parallel.

    # Create a huge query.  We're shooting for more than 2MB; the server buffer
    # size is 1MB and we want to make sure we need more than 2 chunks, and we
    # want to tickle some buffer boundary conditions

    base = 2 * 1024 * 1024
    sizes = list(range(base - 256, base + 256, 63))
    while sizes:
        batch = sizes[:4]
        sizes = sizes[4:]
        suffix = "_".join(map(str, batch))
        setattr(
            original_test_class,
            f"test_bigQuery_{suffix}",
            lambda self, batch=batch: self.do_test_bigQuery(sizes=batch),
        )

    return original_test_class


@WatchmanTestCase.expand_matrix
@populate_tests
class TestBig(WatchmanTestCase.WatchmanTestCase):
    def checkOSApplicability(self) -> None:
        if os.name == "nt":
            self.skipTest("Windows has problems with this test")

    def do_test_bigQuery(self, sizes: List[int]) -> None:
        root = self.mkdtemp()

        self.watchmanCommand("watch", root)

        for size in sizes:
            try:
                res = self.watchmanCommand(
                    "query", root, {"expression": ["name", "a" * size]}
                )

                self.assertEqual([], res["files"])
            except pywatchman.WatchmanError as e:
                # We don't want to print the real command, as
                # it is too long, instead, replace it with
                # a summary of the size that we picked
                e.cmd = "big query with size %d" % size

                if self.transport == "cli":
                    e.cmd = "%s\n%s" % (e.cmd, self.getLogSample())
                raise
