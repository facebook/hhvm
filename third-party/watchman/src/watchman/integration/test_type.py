# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import os

import pywatchman
from watchman.integration.lib import WatchmanTestCase


@WatchmanTestCase.expand_matrix
class TestTypeExpr(WatchmanTestCase.WatchmanTestCase):
    def test_type_expr(self) -> None:
        root = self.mkdtemp()

        self.touchRelative(root, "foo.c")
        os.mkdir(os.path.join(root, "subdir"))
        self.touchRelative(root, "subdir", "bar.txt")

        self.watchmanCommand("watch", root)

        self.assertFileListsEqual(
            self.watchmanCommand(
                "query", root, {"expression": ["type", "f"], "fields": ["name"]}
            )["files"],
            ["foo.c", "subdir/bar.txt"],
        )

        self.assertEqual(
            self.watchmanCommand(
                "query", root, {"expression": ["type", "d"], "fields": ["name", "type"]}
            )["files"],
            [{"name": "subdir", "type": "d"}],
        )

        with self.assertRaises(pywatchman.WatchmanError) as ctx:
            self.watchmanCommand("query", root, {"expression": ["type", "x"]})

        self.assertIn("invalid type string 'x'", str(ctx.exception))

        with self.assertRaises(pywatchman.WatchmanError) as ctx:
            self.watchmanCommand("query", root, {"expression": "type"})

        self.assertIn(
            '"type" term requires a type string parameter', str(ctx.exception)
        )

        with self.assertRaises(pywatchman.WatchmanError) as ctx:
            self.watchmanCommand("query", root, {"expression": ["type", 123]})

        self.assertIn(
            'First parameter to "type" term must be a type string', str(ctx.exception)
        )
