#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import time

from mcrouter.test.MCProcess import Memcached
from mcrouter.test.McrouterTestCase import McrouterTestCase

class TestModifyExptime(McrouterTestCase):
    config = './mcrouter/test/test_modify_exptime.json'

    def setUp(self):
        self.mc = self.add_server(Memcached())
        self.mcr = self.add_mcrouter(self.config)

    def test_modify_negative_exptime(self):
        self.assertTrue(self.mc.set("a", "value"))
        self.assertEqual(self.mcr.get("a"), "value")

        self.assertTrue(self.mcr.set("a", "value2"))
        self.assertIsNone(self.mc.get("a"))
        self.assertIsNone(self.mcr.get("a"))

    def test_modify_infinite_exptime(self):
        self.assertTrue(self.mcr.set("b", "value", exptime=-1))
        self.assertEqual(self.mc.get("b"), "value")
        self.assertEqual(self.mcr.get("b"), "value")

    def test_modify_smaller_exptime(self):
        self.assertTrue(self.mcr.set("c", "value"))
        self.assertEqual(self.mc.get("c"), "value")
        self.assertEqual(self.mcr.get("c"), "value")

        # wait for the value to expire
        time.sleep(5)

        self.assertIsNone(self.mc.get("c"))
        self.assertIsNone(self.mcr.get("c"))

    def test_modify_min_exptime(self):
        self.assertTrue(self.mcr.set("d", "value"))
        self.assertEqual(self.mc.get("d"), "value")
        self.assertEqual(self.mcr.get("d"), "value")

        # wait for the value to expire
        time.sleep(5)

        self.assertIsNone(self.mc.get("d"))
        self.assertIsNone(self.mcr.get("d"))
