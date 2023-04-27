#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

from mcrouter.test.MCProcess import Memcached
from mcrouter.test.McrouterTestCase import McrouterTestCase

MC_MSG_FLAG_BIG_VALUE = 0x8000


class TestBigvalue(McrouterTestCase):
    config = './mcrouter/test/test_bigvalue.json'
    split_size = 5000
    extra_args = ['--big-value-split-threshold', str(split_size),
                  '--big-value-batch-size', '2',
                  '-t', '30']

    def setUp(self):
        self.add_server(Memcached())

    def get_mcrouter(self):
        return self.add_mcrouter(
            self.config,
            extra_args=self.extra_args)

    def test_bigvalue(self):
        mcrouter = self.get_mcrouter()
        # Use a series of characters that would at least somehow produce
        # different pieces. Also test that we can properly handle a case when
        # the last piece is smaller than threshold.
        value = "abc" * (7 * 1000 * 1000 + 5)
        self.assertTrue(mcrouter.set('key', value))
        # Not assertEqual, which would output 20Mb of 'x'
        # if the test fails
        self.assertTrue(mcrouter.get('key') == value)

    def test_bigvalue_leases(self):
        mcrouter = self.get_mcrouter()
        value = 'abc' * (7 * 1000 * 1000 + 5)

        reply = mcrouter.leaseGet('key')
        self.assertEqual(reply['value'], '')
        self.assertTrue(reply['token'] > 1)

        hot_miss_reply = mcrouter.leaseGet('key')
        self.assertEqual(hot_miss_reply['token'], 1)

        self.assertTrue(mcrouter.leaseSet(
            'key',
            {'value': value, 'token': reply['token']},
        ))

        reply = mcrouter.leaseGet('key')
        self.assertEqual(reply['token'], None)
        self.assertEqual(reply['value'], value)

    def test_bigvalue_leases_hit_meta_miss_piece(self):
        mcrouter = self.get_mcrouter()
        value = 'a' * self.split_size + 'a'

        # BigValueRoute will split 'value' into two pieces
        mcrouter.set('key', '1-2-1234', flags=MC_MSG_FLAG_BIG_VALUE)  # metadata
        mcrouter.set('key:0:1234', value[:self.split_size])  # subpiece 1
        mcrouter.set('key:1:1234', value[self.split_size:])  # subpiece 2

        # Check that our manual imitation of BigValueRoute worked
        self.assertEqual(mcrouter.get('key'), value)

        # Ensure a usable lease token is returned when the metadata is present
        # but one of the subpieces is missing
        self.assertTrue(mcrouter.delete('key:1:1234'))
        reply = mcrouter.leaseGet('key')
        self.assertTrue(reply['token'] > 1)
        self.assertEqual(reply['value'], '')

    def test_bigvalue_leases_delete_stale_value(self):
        mcrouter = self.get_mcrouter()
        value = 'a' * self.split_size + 'a'

        # BigValueRoute will split 'value' into two pieces
        mcrouter.set('key', '1-2-1234', flags=MC_MSG_FLAG_BIG_VALUE)  # metadata
        mcrouter.set('key:0:1234', value[:self.split_size])  # subpiece 1
        mcrouter.set('key:1:1234', value[self.split_size:])  # subpiece 2

        # Check that our manual imitation of BigValueRoute worked
        self.assertEqual(mcrouter.get('key'), value)

        # Ensure a usable lease token is returned when the metadata is present
        # but value is empty when the item is deleted
        self.assertTrue(mcrouter.delete('key'))
        reply = mcrouter.leaseGet('key')
        self.assertTrue(reply['token'] > 1)
        self.assertEqual(reply['value'], '')
