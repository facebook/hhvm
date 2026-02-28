# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import json
import os

from watchman.integration.lib import WatchmanTestCase


@WatchmanTestCase.expand_matrix
class TestInfo(WatchmanTestCase.WatchmanTestCase):
    def test_sock_name(self) -> None:
        resp = self.watchmanCommand("get-sockname")
        self.assertEqual(
            resp["sockname"],
            self.watchmanInstance().getSockPath().legacy_sockpath(),
        )

    def test_get_config_empty(self) -> None:
        root = self.mkdtemp()
        self.watchmanCommand("watch", root)
        self.assertEqual(self.get_config(root), {})

    def test_get_config(self) -> None:
        config = {"test-key": "test-value"}
        root = self.mkdtemp()
        with open(os.path.join(root, ".watchmanconfig"), "w") as f:
            json.dump(config, f)
        self.watchmanCommand("watch", root)
        self.assertEqual(self.get_config(root), config)

    def get_config(self, root):
        resp = self.watchmanCommand("get-config", root)
        config = resp["config"]
        return config
