# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import pywatchman
from watchman.integration.lib import WatchmanTestCase


@WatchmanTestCase.expand_matrix
class TestInvalidExpr(WatchmanTestCase.WatchmanTestCase):
    def test_invalid_expr_term(self) -> None:
        root = self.mkdtemp()
        self.watchmanCommand("watch", root)

        with self.assertRaises(pywatchman.WatchmanError) as ctx:
            self.watchmanCommand(
                "query",
                root,
                {
                    "expression": [
                        "allof",
                        "dont-implement-this-term",
                        ["anyof", ["suffix", "apcarc"]],
                    ]
                },
            )

        self.assertIn(
            (
                "failed to parse query: unknown expression "
                "term 'dont-implement-this-term'"
            ),
            str(ctx.exception),
        )

    def test_invalid_sync_timeout(self) -> None:
        root = self.mkdtemp()
        self.watchmanCommand("watch", root)

        with self.assertRaises(pywatchman.WatchmanError) as ctx:
            self.watchmanCommand(
                "query", root, {"expression": ["exists"], "sync_timeout": -1}
            )

        self.assertIn(
            "failed to parse query: sync_timeout must be an integer value >= 0",
            str(ctx.exception),
        )

        res = self.watchmanCommand(
            "query", root, {"expression": ["exists"], "sync_timeout": 2000}
        )
        self.assertEqual(res["files"], [])
