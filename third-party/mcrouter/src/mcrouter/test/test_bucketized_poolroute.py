#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

from mcrouter.test.MCProcess import  Memcached
from mcrouter.test.McrouterTestCase import McrouterTestCase


class TestBucketizedPoolRoute(McrouterTestCase):
    config = "./mcrouter/test/mcrouter_test_bucketized_poolroute_bucketized.json"
    extra_args = []

    def setUp(self):
        # The order here corresponds to the order of hosts in the .json
        self.mc1 = self.add_server(Memcached())
        self.mc2 = self.add_server(Memcached())
        self.mc3 = self.add_server(Memcached())
        self.mc4 = self.add_server(Memcached())
        self.mc5 = self.add_server(Memcached())

    def get_mcrouter(self, additional_args=()):
        extra_args = self.extra_args[:]
        extra_args.extend(additional_args)
        return self.add_mcrouter(self.config, extra_args=extra_args)

    def test_bucketized_pool_route(self):
        mcr = self.get_mcrouter()
        key = "key1"
        val = "value1"
        self.assertTrue(mcr.set(key, val))
        self.assertEqual(mcr.get(key), val)
        # todo key1 maps to bucketId 808 and server #_ with bucketization
        self.assertFalse(self.mc1.get(key))
        self.assertFalse(self.mc2.get(key))
        self.assertEqual(self.mc3.get(key), val)
        self.assertFalse(self.mc4.get(key))
        self.assertFalse(self.mc5.get(key))

class TestNonBucketizedPoolRoute(McrouterTestCase):
    config = "./mcrouter/test/mcrouter_test_bucketized_poolroute_nonbucketized.json"
    extra_args = []

    def setUp(self):
        # The order here corresponds to the order of hosts in the .json
        self.mc1 = self.add_server(Memcached())
        self.mc2 = self.add_server(Memcached())
        self.mc3 = self.add_server(Memcached())
        self.mc4 = self.add_server(Memcached())
        self.mc5 = self.add_server(Memcached())

    def get_mcrouter(self, additional_args=()):
        extra_args = self.extra_args[:]
        extra_args.extend(additional_args)
        return self.add_mcrouter(self.config, extra_args=extra_args)

    def test_bucketized_pool_route(self):
        mcr = self.get_mcrouter()
        key = "key1"
        val = "value1"
        self.assertTrue(mcr.set(key, val))
        self.assertEqual(mcr.get(key), val)
        # key1 maps to server #3 without bucketization
        self.assertFalse(self.mc1.get(key))
        self.assertFalse(self.mc2.get(key))
        self.assertEqual(self.mc3.get(key), val)
        self.assertFalse(self.mc4.get(key))
        self.assertFalse(self.mc5.get(key))
