#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.


from mcrouter.test.MCProcess import Memcached
from mcrouter.test.McrouterTestCase import McrouterTestCase
from mcrouter.test.mock_servers import CustomErrorServer


class TestCustomFailover(McrouterTestCase):
    config = './mcrouter/test/test_custom_failover.json'
    extra_args = []

    def setUp(self):
        self.mc1 = self.add_server(CustomErrorServer())
        self.mc2 = self.add_server(Memcached())
        self.mcr = self.add_mcrouter(self.config, extra_args=self.extra_args)

    def test_failover(self):
        self.assertTrue(self.mc2.set("key", "value1"))
        self.assertEqual(self.mcr.get("key"), "value1")

    def test_failover_no(self):
        # should not failover tko error
        self.mc1.terminate()

        self.assertTrue(self.mc2.set("key", "value1"))
        self.assertEqual(self.mcr.get("key"), None)

class TestCustomFailoverOverride(McrouterTestCase):
    config = './mcrouter/test/test_custom_failover_override.json'
    extra_args = []

    def setUp(self):
        self.mc1 = self.add_server(CustomErrorServer())
        self.mc2 = self.add_server(Memcached())

        self.mcr = self.add_mcrouter(self.config, extra_args=self.extra_args)

    def test_failover_gets(self):
        self.assertTrue(self.mc2.set("key", "value1"))
        self.assertEqual(self.mcr.get("key"), "value1")

    def test_failover_updates(self):
        # should not failover anything
        self.assertFalse(self.mcr.set("key", "value2"))
