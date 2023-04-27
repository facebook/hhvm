#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

from mcrouter.test.MCProcess import Memcached
from mcrouter.test.McrouterTestCase import McrouterTestCase

class TestConstShardHash(McrouterTestCase):
    config = './mcrouter/test/test_const_shard_hash.json'
    extra_args = []

    def test_const_shard_hash(self):
        mc1 = self.add_server(Memcached())
        mc2 = self.add_server(Memcached())
        mcrouter = self.add_mcrouter(self.config, extra_args=self.extra_args)

        key = 'foo:0:test'
        value = 'value0'
        mcrouter.set(key, value)
        self.assertEqual(mc1.get(key), value)
        self.assertIsNone(mc2.get(key))
        key = 'foo:1:test'
        value = 'value1'
        mcrouter.set(key, value)
        self.assertIsNone(mc1.get(key))
        self.assertEqual(mc2.get(key), value)
