#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

from mcrouter.test.MCProcess import Memcached
from mcrouter.test.McrouterTestCase import McrouterTestCase

class TestMigratedFailover(McrouterTestCase):
    config = './mcrouter/test/test_migrated_failover.json'
    extra_args = [
        '--probe-timeout-initial=100',
        '--probe-timeout-max=100'
    ]

    def get_mcrouter(self):
        return self.add_mcrouter(self.config, extra_args=self.extra_args)

    def test_migrated_failover(self):
        self.add_server(Memcached())  # "old" pool, ignored
        mc_a = self.add_server(Memcached())
        self.add_server(Memcached())  # "old" pool, ignored
        mc_b = self.add_server(Memcached())

        mc_a.set("key", "a")
        mc_b.set("key", "b")

        mcrouter = self.get_mcrouter()

        self.assertEqual("a", mcrouter.get("key"))

        mc_a.terminate()

        self.assertEqual("b", mcrouter.get("key"))
