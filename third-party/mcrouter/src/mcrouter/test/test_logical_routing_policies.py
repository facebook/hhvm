#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

from mcrouter.test.MCProcess import *
from mcrouter.test.McrouterTestCase import McrouterTestCase

class TestLogicalRoutingPolicies(McrouterTestCase):
    config = './mcrouter/test/test_logical_routing_policies.json'
    extra_args = []

    def setUp(self):
        self.mc = self.add_server(Memcached())

    def test_different_cluster(self):
        mcrouter = self.add_mcrouter(self.config, '/region1/cluster2/',
                                     extra_args=self.extra_args)
        key = 'foo1'
        value = 'value1'
        mcrouter.set(key, value)
        self.assertEqual(self.mc.get(key), value)

    def test_different_region_cluster(self):
        mcrouter = self.add_mcrouter(self.config, '/region2/cluster3/',
                                     extra_args=self.extra_args)
        key = 'foo2'
        value = 'value2'
        mcrouter.set(key, value)
        self.assertEqual(self.mc.get(key), value)
