#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import time

from mcrouter.test.McrouterTestCase import McrouterTestCase
from mcrouter.test.MCProcess import Memcached

class TestShardSplits(McrouterTestCase):
    config = './mcrouter/test/test_shard_splits.json'
    extra_args = []

    def setUp(self):
        for _ in range(3):
            self.add_server(Memcached())

    def test_shard_splits_basic(self):
        mcrouter = self.add_mcrouter(
            self.config,
            extra_args=self.extra_args)

        # Test set, get, direct get (by split shard id). Vanilla set goes to the
        # primary host by default
        self.assertTrue(mcrouter.set('a:1:blah', 'value'))
        self.assertTrue(mcrouter.set('a:1aa:blah', 'value_aa'))
        self.assertTrue(mcrouter.set('a:1ba:blah', 'value_ba'))
        time.sleep(0.1)
        # Get without specifying shard copy goes to a random copy
        r = mcrouter.get('a:1:blah')
        self.assertTrue(r == 'value' or r == 'value_aa' or r == 'value_ba')
        self.assertEqual(mcrouter.get('a:1aa:blah'), 'value_aa')
        self.assertEqual(mcrouter.get('a:1ba:blah'), 'value_ba')
        self.assertIsNone(mcrouter.get('a:1ca:blah'))

    def test_shard_splits_update(self):
        mcrouter = self.add_mcrouter(
            self.config,
            extra_args=self.extra_args)

        # set & delete with splits
        self.assertTrue(mcrouter.set('a:1:foo', 'value'))
        self.assertTrue(mcrouter.set('a:1aa:foo', 'value'))
        self.assertTrue(mcrouter.set('a:1ba:foo', 'value'))
        time.sleep(0.1)
        self.assertEqual(mcrouter.get('a:1:foo'), 'value')
        self.assertEqual(mcrouter.get('a:1aa:foo'), 'value')
        self.assertEqual(mcrouter.get('a:1ba:foo'), 'value')

        mcrouter.delete('a:1:foo')
        time.sleep(0.1)
        self.assertIsNone(mcrouter.get('a:1:foo'))
        self.assertIsNone(mcrouter.get('a:1aa:foo'))
        self.assertIsNone(mcrouter.get('a:1ba:foo'))

        # No splits
        self.assertTrue(mcrouter.set('a:5:bar', 'value2'))
        self.assertEqual(mcrouter.get('a:5:bar'), 'value2')

        self.assertTrue(mcrouter.delete('a:5:bar'))
        self.assertIsNone(mcrouter.get('a:5:bar'))

        # Arithmetic operations with splits
        self.assertTrue(mcrouter.set('a:1:counter', '5'))
        time.sleep(0.1)
        self.assertEqual(mcrouter.incr('a:1:counter'), 6)
        time.sleep(0.1)
        self.assertEqual(mcrouter.get('a:1:counter'), '6')

        self.assertEqual(mcrouter.decr('a:1:counter', 3), 3)
        time.sleep(0.1)
        self.assertEqual(mcrouter.get('a:1:counter'), '3')

        # Arithmetic operations without splits
        self.assertTrue(mcrouter.set('a:125:counter', '125'))
        self.assertEqual(mcrouter.incr('a:125:counter'), 126)
        self.assertEqual(mcrouter.get('a:125:counter'), '126')
        self.assertIsNone(mcrouter.get('a:125aa:counter'))

        self.assertEqual(mcrouter.decr('a:125:counter', 124), 2)
        self.assertEqual(mcrouter.get('a:125:counter'), '2')
        self.assertIsNone(mcrouter.get('a:125aa:counter'))
