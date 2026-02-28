# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import os
import os.path

import pywatchman
from watchman.integration.lib import WatchmanTestCase
from watchman.integration.lib.path_utils import norm_absolute_path


@WatchmanTestCase.expand_matrix
class TestAbsoluteRoot(WatchmanTestCase.WatchmanTestCase):
    def test_dot(self) -> None:
        root = self.mkdtemp()

        save_dir = os.getcwd()
        try:
            os.chdir(root)

            dot = "" if os.name == "nt" else "."

            if self.transport == "cli":
                res = self.watchmanCommand("watch", dot)
                self.assertEqual(root, norm_absolute_path(res["watch"]))
            else:
                with self.assertRaises(pywatchman.WatchmanError) as ctx:
                    self.watchmanCommand("watch", dot)

                self.assertIn("must be absolute", str(ctx.exception))

        finally:
            os.chdir(save_dir)

    def test_root(self) -> None:
        if os.name != "nt":
            with self.assertRaises(pywatchman.WatchmanError) as ctx:
                self.watchmanCommand("watch", "/")

                self.assertIn("cannot watch", str(ctx.exception))
