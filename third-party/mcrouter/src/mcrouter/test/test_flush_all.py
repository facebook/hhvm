#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import time

from mcrouter.test.MCProcess import MockMemcached
from mcrouter.test.McrouterTestCase import McrouterTestCase

class TestMcrouterRoutingPrefixAscii(McrouterTestCase):
    config = './mcrouter/test/mcrouter_test_basic_2_1_1.json'

    def setUp(self):
        # The order here corresponds to the order of hosts in the .json
        self.mc1 = self.add_server(MockMemcached())
        self.mc2 = self.add_server(MockMemcached())

    def get_mcrouter(self, enableFlush):
        args = ['--server-timeout', '2000']
        if enableFlush:
            args.append('--enable-flush-cmd')
        return self.add_mcrouter(self.config, '/a/a/', extra_args=args)

    def test_flush_all_disabled(self):
        mcr = self.get_mcrouter(enableFlush=False)
        self.assertEqual(mcr.flush_all(), "SERVER_ERROR Command disabled")

    def test_flush_all(self):
        mcr = self.get_mcrouter(enableFlush=True)

        self.assertTrue(mcr.set("key", "value"))
        self.assertEqual(mcr.get("key"), "value")

        self.assertTrue(mcr.set("/b/b/key", "value2"))
        self.assertEqual(mcr.get("/b/b/key"), "value2")

        self.assertEqual(mcr.flush_all(), "OK")

        self.assertIsNone(mcr.get("key"))
        self.assertIsNone(mcr.get("/b/b/key"))

        # delay
        self.assertTrue(mcr.set("key2", "value2"))
        self.assertEqual(mcr.get("key2"), "value2")

        start = time.time()
        self.assertEqual(mcr.flush_all(1), "OK")
        # it should take at least 1 second
        self.assertGreater(time.time() - start, 0.5)

        # flush_all is syncronous
        self.assertIsNone(mcr.get("key2"))
