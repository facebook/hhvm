#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

from mcrouter.test.MCProcess import Memcached
from mcrouter.test.McrouterTestCase import McrouterTestCase
import time


class TestAllowGetsOnly(McrouterTestCase):
    config = './mcrouter/test/mcrouter_test_basic_1_1_1.json'

    def setUp(self):
        self.mc = self.add_server(Memcached())
        extra_args = ['--allow-only-gets']
        self.mcr = self.add_mcrouter(self.config, extra_args=extra_args)

    def test_allow_gets_only(self):
        key_set_time = int(time.time())
        self.assertTrue(self.mc.set('key', '1', exptime=100))
        key_after_set_time = int(time.time())

        self.assertEqual(self.mc.get('key'), '1')

        self.assertFalse(self.mcr.set('key', '2'))
        self.assertFalse(self.mcr.delete('key'))
        self.assertFalse(self.mcr.incr('key'))
        self.assertFalse(self.mcr.decr('key'))
        self.assertFalse(self.mcr.add('key', '2'))
        self.assertFalse(self.mcr.add('key2', '1'))

        # both get and metaget should work
        self.assertEqual(self.mcr.get('key'), '1')
        self.assertIn(int(self.mcr.metaget('key')['exptime']),
                range(key_set_time + 100, key_after_set_time + 101))
        self.assertIsNone(self.mcr.get('key2'))
        # gat and gats should work the same as get/gets
        self.assertEqual(self.mcr.gat(0, 'key'), '1')
        self.assertIsNone(self.mcr.gat(0, 'key2'))
        self.assertIsNotNone(self.mcr.gats(0, 'key'))
        self.assertIsNone(self.mcr.gat(0, 'key2'))
