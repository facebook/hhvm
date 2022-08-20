#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.


import time

from mcrouter.test.MCProcess import Memcached
from mcrouter.test.McrouterTestCase import McrouterTestCase
from mcrouter.test.mock_servers import SleepServer
from mcrouter.test.mock_servers import ConnectionErrorServer


class TestDevNull(McrouterTestCase):
    config = './mcrouter/test/test_dev_null.json'
    extra_args = []

    def setUp(self):
        # The order here must corresponds to the order of hosts in the .json
        self.mc_good = self.add_server(Memcached())
        self.mc_wild = self.add_server(Memcached())

    def get_mcrouter(self):
        return self.add_mcrouter(self.config, extra_args=self.extra_args)

    def test_dev_null(self):
        mcr = self.get_mcrouter()

        # finally setup is done
        mcr.set("good:key", "should_be_set")
        mcr.set("key", "should_be_set_wild")
        mcr.set("null:key", "should_not_be_set")
        mcgood_val = self.mc_good.get("good:key")
        mcnull_val = self.mc_wild.get("null:key")
        mcwild_val = self.mc_wild.get("key")

        self.assertEqual(mcgood_val, "should_be_set")
        self.assertEqual(mcnull_val, None)
        self.assertEqual(mcwild_val, "should_be_set_wild")

        self.assertEqual(mcr.delete("null:key2"), None)
        self.assertEqual(int(mcr.stats('all')['dev_null_requests']), 2)


class TestDuplicateServers(McrouterTestCase):
    config = './mcrouter/test/test_duplicate_servers.json'
    extra_args = []

    def setUp(self):
        self.wildcard = self.add_server(Memcached(), 12345)

    def get_mcrouter(self):
        return self.add_mcrouter(
            self.config, '/a/a/', extra_args=self.extra_args)

    def test_duplicate_servers(self):
        mcr = self.get_mcrouter()

        stats = mcr.stats('servers')
        # Check that only one proxy destination connection is made
        # for all the duplicate servers
        self.assertEqual(1, len(stats))
        # Hardcoding default server timeout
        key = ('localhost:' + str(self.port_map[12345]) +
               ':ascii:plain:notcompressed-1000')
        self.assertTrue(key in stats)


class TestDuplicateServersDiffTimeouts(McrouterTestCase):
    config = './mcrouter/test/test_duplicate_servers_difftimeouts.json'
    extra_args = []

    def setUp(self):
        self.wildcard = self.add_server(Memcached(), 12345)

    def get_mcrouter(self):
        return self.add_mcrouter(
            self.config, '/a/a/', extra_args=self.extra_args)

    def test_duplicate_servers_difftimeouts(self):
        mcr = self.get_mcrouter()

        stats = mcr.stats('servers')
        # Check that only two proxy destination connections are made
        # for all the duplicate servers in pools with diff timeout
        self.assertEqual(2, len(stats))
        # Hardcoding default server timeout
        key = ('localhost:' + str(self.port_map[12345]) +
               ':ascii:plain:notcompressed-1000')
        self.assertTrue(key in stats)

        key = ('localhost:' + str(self.port_map[12345]) +
               ':ascii:plain:notcompressed-2000')
        self.assertTrue(key in stats)


class TestPoolServerErrors(McrouterTestCase):
    config = './mcrouter/test/test_pool_server_errors.json'

    def setUp(self):
        self.mc1 = self.add_server(Memcached())
        # mc2 is ErrorRoute
        self.mc3 = self.add_server(Memcached())

    def test_pool_server_errors(self):
        mcr = self.add_mcrouter(self.config, '/a/a/')
        self.assertIsNone(mcr.get('test'))

        stats = mcr.stats('servers')
        self.assertEqual(2, len(stats))

        self.assertTrue(mcr.set('/b/b/abc', 'valueA'))
        self.assertEqual(self.mc1.get('abc'), 'valueA')

        self.assertFalse(mcr.set('/b/b/a', 'valueB'))

        self.assertTrue(mcr.set('/b/b/ab', 'valueC'))
        self.assertEqual(self.mc3.get('ab'), 'valueC')
