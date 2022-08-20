#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.


import time

from mcrouter.test.MCProcess import Memcached
from mcrouter.test.McrouterTestCase import McrouterTestCase


class TestShadowRoute(McrouterTestCase):
    config = './mcrouter/test/test_shadow_route.json'

    def setUp(self):
        # The order here must corresponds to the order of hosts in the .json
        self.mc_foo_0 = self.add_server(Memcached())
        self.mc_foo_1 = self.add_server(Memcached())
        self.mc_foo_shadow = self.add_server(Memcached())
        self.mc_bar_shadow = self.add_server(Memcached())
        self.mcrouter = self.add_mcrouter(self.config)

    def test_shadow_route(self):
        shadow_list = [5, 7, 13, 33, 43, 46, 58, 71, 83, 85, 89, 91, 93]
        kv = [('f' + str(i), 'value' + str(i)) for i in range(100)]
        shadow_keys = [kv[i][0] for i in shadow_list]

        for key, value in kv:
            self.mcrouter.set(key, value)

        retries = 0
        key, val = kv[0]
        while self.mcrouter.get(key) != val and retries < 5:
            time.sleep(1)
            retries += 1

        for key, value in kv:
            self.assertEqual(self.mc_foo_0.get(key), value)
            self.assertEqual(self.mc_foo_1.get(key), value)
            if key in shadow_keys:
                self.assertEqual(self.mc_foo_shadow.get(key), value)
                self.assertEqual(self.mc_bar_shadow.get(key), value)
            else:
                self.assertIsNone(self.mc_foo_shadow.get(key))
                self.assertIsNone(self.mc_bar_shadow.get(key))

    def test_shadow_route_leases(self):
        kv = [('g' + str(i), 'value' + str(i)) for i in range(1, 20)]

        # Send a few lease-get misses to the shadow in order to ensure primary
        # and shadow hosts don't issue lease tokens in lockstep later on.
        current_shadow_token = 0
        for i in range(100, 500):
            k = 'warmup' + str(i)
            current_shadow_token = self.mc_foo_shadow.leaseGet(k)['token']

        self.assertGreater(
            current_shadow_token,
            self.mc_foo_0.leaseGet('a')['token'],
        )
        self.assertGreater(
            current_shadow_token,
            self.mc_foo_1.leaseGet('a')['token'],
        )

        for key, value in kv:
            self.assertIsNone(self.mc_foo_0.get(key))
            self.assertIsNone(self.mc_foo_1.get(key))
            self.assertIsNone(self.mc_foo_shadow.get(key))

            token = self.mcrouter.leaseGet(key)['token']
            self.assertGreater(token, 1)

            self.assertTrue(self.mcrouter.leaseSet(
                key,
                {'value': value, 'token': token},
            ))
            self.assertTrue(self.mcrouter.set(key, value))

            self.assertEqual(
                self.mc_foo_0.get(key) or self.mc_foo_1.get(key),
                value,
            )
            self.assertEqual(self.mc_foo_shadow.get(key), value)
