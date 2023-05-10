#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

from mcrouter.test.MCProcess import Memcached
from mcrouter.test.McrouterTestCase import McrouterTestCase

class TestMcRefill(McrouterTestCase):
    config = './mcrouter/test/test_mcrefill.json'
    extra_args = []

    def setUp(self):
        self.mc1 = self.add_server(Memcached())
        self.mc2 = self.add_server(Memcached())
        self.mcrouter = self.add_mcrouter(
            self.config,
            extra_args=self.extra_args)

    def test_basic(self):
        key = 'foo'
        value = 'value'
        # Set to refill tier
        self.mc2.set(key, value)
        self.assertEqual(self.mc2.get(key), value)
        # Get through mcrouter and check its refilled
        self.assertNotEqual(self.mcrouter.get(key), value)
        def condition():
            stat = self.mc1.stats()
            return stat['total_items'] == '1'
        self.assert_eventually_true(condition)
        self.assertEqual(self.mcrouter.get(key), value)


    def test_basic_stats(self):
        key = 'foo'
        value = 'value'
        # Set to refill tier
        self.mc2.set(key, value)
        self.assertEqual(self.mc2.get(key), value)
        stat2 = self.mc2.stats()
        self.assertEqual(stat2['total_items'], '1')
        # Get through mcrouter and check its refilled
        stat1 = self.mc1.stats()
        self.assertEqual(stat1['total_items'], '0')
        self.assertNotEqual(self.mcrouter.get(key), value)
        def condition():
            stat = self.mc1.stats()
            return stat['total_items'] == '1'
        self.assert_eventually_true(condition)
        self.assertEqual(self.mcrouter.get(key), value)


    def test_basic_leases(self):
        key = 'foo'
        value = 'value'
        # Set to refill tier
        self.mc2.set(key, value)
        self.assertEqual(self.mc2.get(key), value)
        # Get through mcrouter and check its refilled
        res = self.mcrouter.leaseGet(key)
        self.assertNotEqual(res['value'], value)
        def condition():
            stat = self.mc1.stats()
            return stat['total_items'] == '1'
        self.assert_eventually_true(condition)
        res = self.mcrouter.leaseGet(key)
        self.assertEqual(res['value'], value)


    def test_gets_stats(self):
        key = 'foo'
        value = 'value'
        # Set to refill tier
        self.mc2.set(key, value)
        self.assertEqual(self.mc2.get(key), value)
        stat2 = self.mc2.stats()
        self.assertEqual(stat2['total_items'], '1')
        # Get through mcrouter and check its refilled
        stat1 = self.mc1.stats()
        self.assertEqual(stat1['total_items'], '0')
        self.assertIsNone(self.mcrouter.gets(key))
        def condition():
            stat = self.mc1.stats()
            return stat['total_items'] == '1'
        self.assert_eventually_true(condition)
        self.assertEqual(self.mcrouter.gets(key)["value"], value)
