#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.


import time

from mcrouter.test.MCProcess import Memcached
from mcrouter.test.McrouterTestCase import McrouterTestCase


class TestMcrouterRoutingPrefixAscii(McrouterTestCase):
    config = './mcrouter/test/routing_prefix_test_ascii.json'
    extra_args = []

    def setUp(self):
        # The order here must corresponds to the order of hosts in the .json
        self.allhosts = []
        for _ in range(0, 4):
            self.allhosts.append(self.add_server(Memcached()))

    def get_mcrouter(self):
        return self.add_mcrouter(
            self.config, '/a/a/', extra_args=self.extra_args)

    def test_routing_prefix(self):
        mcr = self.get_mcrouter()
        nclusters = len(self.allhosts)

        # first try setting a key to the local cluster
        mcr.set("testkeylocal", "testvalue")
        self.assertEqual(self.allhosts[0].get("testkeylocal"), "testvalue")
        for i in range(1, nclusters):
            self.assertIsNone(self.allhosts[i].get("testkeylocal"))

        mcr.set("/*/*/testkey-routing", "testvalue")
        # /*/*/ is all-fastest, and some requests might complete asynchronously.
        # As a workaround, just wait
        time.sleep(1)

        local = self.allhosts[0].get("testkey-routing", True)
        self.assertEqual(local["value"], "testvalue")
        # make sure the key got set as "/*/*/key"
        for i in range(1, nclusters):
            local = self.allhosts[i].get("/*/*/testkey-routing", True)
            self.assertEqual(local["value"], "testvalue")


class TestMcrouterRoutingPrefixCaret(TestMcrouterRoutingPrefixAscii):
    config = './mcrouter/test/routing_prefix_test_caret.json'


class TestMcrouterRoutingPrefixOldNaming(TestMcrouterRoutingPrefixAscii):
    config = './mcrouter/test/routing_prefix_test_old_naming.json'


class TestMcrouterRoutingPrefixSimpleRoutes(TestMcrouterRoutingPrefixAscii):
    config = './mcrouter/test/routing_prefix_test_simple_routes.json'


class TestFallbackRouting(McrouterTestCase):
    config = './mcrouter/test/routing_prefix_test_fallback_route.json'
    extra_args = []

    def setUp(self):
        self.aa = self.add_server(Memcached())
        self.ab = self.add_server(Memcached())
        self.ba = self.add_server(Memcached())
        self.bb = self.add_server(Memcached())

    def get_mcrouter(self):
        return self.add_mcrouter(
            self.config, '/a/a/', extra_args=self.extra_args)

    def test_fallback_routing(self):
        mcr = self.get_mcrouter()

        key = "/*/*/key"
        orig_value = "orig"

        mcr.set(key, orig_value)
        time.sleep(1)
        self.assertEqual(self.aa.get('key'), orig_value)
        self.assertEqual(self.ab.get('key'), orig_value)
        self.assertEqual(self.ba.get('key'), orig_value)
        self.assertEqual(self.bb.get('key'), orig_value)

        key = "/a/foobar/key"
        value1 = "value1"
        mcr.set(key, value1)
        time.sleep(1)
        self.assertEqual(self.ab.get('key'), orig_value)
        self.assertEqual(self.ba.get('key'), orig_value)
        self.assertEqual(self.bb.get('key'), orig_value)
        self.assertEqual(self.aa.get('key'), value1)


class TestCustomRoutingPrefixes(McrouterTestCase):
    config = './mcrouter/test/routing_prefix_test_custom.json'
    extra_args = []

    def setUp(self):
        self.aa = self.add_server(Memcached())
        self.ab = self.add_server(Memcached())
        self.ba = self.add_server(Memcached())
        self.bb = self.add_server(Memcached())

    def get_mcrouter(self):
        return self.add_mcrouter(
            self.config, '/a/a/', extra_args=self.extra_args)

    def test_custom_routing_prefix(self):
        mcr = self.get_mcrouter()

        key = "/*/a/key"
        value = "value"

        mcr.set(key, value)
        time.sleep(1)
        self.assertEqual(self.aa.get('key'), value)
        self.assertEqual(self.ba.get('key'), value)

        key = "/b*/*/key"
        value = "value2"
        mcr.set(key, value)
        time.sleep(1)
        self.assertEqual(self.ba.get('key'), value)
        self.assertEqual(self.bb.get('key'), value)

        key = "/b/*b*/key"
        value = "value3"
        mcr.set(key, value)
        self.assertEqual(self.bb.get('key'), value)
