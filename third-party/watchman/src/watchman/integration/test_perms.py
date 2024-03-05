# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import os
import unittest

import pywatchman
from watchman.integration.lib import WatchmanTestCase


def is_root() -> bool:
    return hasattr(os, "geteuid") and os.geteuid() == 0


@WatchmanTestCase.expand_matrix
class TestPerms(WatchmanTestCase.WatchmanTestCase):
    def checkOSApplicability(self) -> None:
        if os.name == "nt":
            self.skipTest("N/A on Windows")

    @unittest.skipIf(is_root(), "N/A if root")
    def test_permDeniedSubDir(self) -> None:
        root = self.mkdtemp()
        subdir = os.path.join(root, "subdir")
        os.mkdir(subdir)
        os.chmod(subdir, 0)
        self.watchmanCommand("watch", root)
        res = self.watchmanCommand(
            "query", root, {"expression": ["exists"], "fields": ["name"]}
        )
        self.assertRegex(res["warning"], "Marking this portion of the tree deleted")

    @unittest.skipIf(is_root(), "N/A if root")
    def test_permDeniedRoot(self) -> None:
        root = self.mkdtemp()
        os.chmod(root, 0)
        with self.assertRaisesRegex(pywatchman.CommandError, "(open|opendir|realpath)"):
            self.watchmanCommand("watch", root)
