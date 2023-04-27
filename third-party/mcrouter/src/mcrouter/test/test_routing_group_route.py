# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals

from mcrouter.test.MCProcess import Memcached
from mcrouter.test.McrouterTestCase import McrouterTestCase


class TestRoutingGroupRoute(McrouterTestCase):
    config = './mcrouter/test/test_routing_group_route.json'
    extra_args = []

    def setUp(self):
        self.memcached_get = self.add_server(Memcached())
        self.memcached_set = self.add_server(Memcached())
        self.memcached_delete = self.add_server(Memcached())
        self.memcached_arithmetic = self.add_server(Memcached())

        self.mcrouter = self.add_mcrouter(
            self.config,
            extra_args=self.extra_args)

    def test_get(self):
        self.assertTrue(self.memcached_get.set('key_get', 'val_get'))
        self.assertEqual('val_get', self.mcrouter.get('key_get'))

    def test_set(self):
        self.assertTrue(self.mcrouter.set('key_set', 'val_set'))
        self.assertEqual('val_set', self.memcached_set.get('key_set'))

    def test_delete(self):
        self.assertTrue(self.memcached_delete.set('key_del', 'val_del'))
        self.assertTrue(self.mcrouter.delete('key_del'))
        self.assertFalse(self.memcached_delete.get('key_del'))

    def test_arith(self):
        self.assertTrue(self.memcached_arithmetic.set('key_arith', '5'))
        self.assertTrue(self.mcrouter.incr('key_arith', 2))
        self.assertEqual(self.memcached_arithmetic.get('key_arith'), '7')


class TestRoutingGroupRouteIncomplete(McrouterTestCase):
    '''
    "default_policy": "ErrorRoute|A",
    "routing_group_policies": {
        "get_like": "PoolRoute|B",
        "update_like": "PoolRoute|B"
    }
    '''
    config = './mcrouter/test/test_routing_group_route_incomplete.json'
    extra_args = []

    def setUp(self):
        self.memcached_A = self.add_server(Memcached())
        self.memcached_B = self.add_server(Memcached())

        self.mcrouter = self.add_mcrouter(self.config, extra_args=self.extra_args)

    def test_set_get(self):
        self.assertTrue(self.mcrouter.set('key0', 'val0'))
        self.assertEqual('val0', self.memcached_B.get('key0'))
        self.assertEqual('val0', self.mcrouter.get('key0'))

    def test_delete(self):
        self.assertTrue(self.memcached_A.set('key0', 'val0'))
        self.mcrouter.delete('key0')
        self.assertFalse(self.memcached_A.get('key0'))

    def test_arith(self):
        self.assertTrue(self.memcached_A.set('key0', '2'))
        self.mcrouter.incr('key0', 1)
        self.assertEqual('3', self.memcached_A.get('key0'))
